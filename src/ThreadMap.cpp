#include "ThreadMap.h"

#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <vector>

// debug
#include "Log.h"

inline void radixSort(ThreadMap& map, ThreadPool& threadPool) {
    const size_t n = map.tileMap.size();
    const int threadCount = ThreadSettings::THREAD_COUNT;

    // Shared histograms for each thread
    std::vector<std::vector<uint32_t>> threadHistogramsLow(threadCount, std::vector<uint32_t>(256, 0));
    std::vector<std::vector<uint32_t>> threadHistogramsHigh(threadCount, std::vector<uint32_t>(256, 0));

    auto computeHistogram = [&](int threadId, size_t start, size_t end) {
        auto& localLow = threadHistogramsLow[threadId];
        auto& localHigh = threadHistogramsHigh[threadId];
        for (size_t i = start; i < end; ++i) {
            uint8_t lowByte = map.tileMap[i] & 0xFF;
            uint8_t highByte = (map.tileMap[i] >> 8) & 0xFF;
            localLow[lowByte]++;
            localHigh[highByte]++;
        }
    };

    auto prefixSum = [](std::vector<uint32_t>& histogram) {
        uint32_t sum = 0;
        for (size_t i = 0; i < histogram.size(); ++i) {
            uint32_t count = histogram[i];
            histogram[i] = sum;
            sum += count;
        }
    };

    auto sortPhase = [&](std::vector<uint32_t>& histogram, size_t start, size_t end, bool useLowBits) {
        for (size_t i = start; i < end; ++i) {
            uint16_t val = map.tileMap[i];
            uint8_t digit = useLowBits ? (val & 0xFF) : ((val >> 8) & 0xFF);
            size_t pos = histogram[digit]++;
            map.tmpTile[pos] = val;
            map.tmpId[pos] = map.idMap[i]; // Mimic operation on secondary vector
        }
    };

    // Divide the array into chunks and process in parallel
    size_t chunkSize = (n + threadCount - 1) / threadCount; // Ensure all elements are covered
    for (int i = 0; i < threadCount; ++i) {
        size_t start = i * chunkSize;
        size_t end = std::min(start + chunkSize, n);
        threadPool.enqueue(i, computeHistogram, i, start, end);
    }

    // Wait for histogram computation
    threadPool.await();

    // Merge histograms
    std::vector<uint32_t> globalHistogramLow(256, 0);
    std::vector<uint32_t> globalHistogramHigh(256, 0);

    for (int i = 0; i < threadCount; ++i) {
        for (size_t j = 0; j < 256; ++j) {
            globalHistogramLow[j] += threadHistogramsLow[i][j];
            globalHistogramHigh[j] += threadHistogramsHigh[i][j];
        }
    }

    // Compute prefix sums
    prefixSum(globalHistogramLow);
    prefixSum(globalHistogramHigh);

    // Sort by lower 8 bits
    for (int i = 0; i < threadCount; ++i) {
        size_t start = i * chunkSize;
        size_t end = std::min(start + chunkSize, n);
        threadPool.enqueue(i, sortPhase, globalHistogramLow, start, end, true);
    }

    // Wait for the first pass to complete
    threadPool.await();

    // Swap primary and secondary arrays for the next pass
    std::swap(map.tileMap, map.tmpTile);
    std::swap(map.idMap, map.tmpId);

    // Sort by upper 8 bits
    for (int i = 0; i < threadCount; ++i) {
        size_t start = i * chunkSize;
        size_t end = std::min(start + chunkSize, n);
        threadPool.enqueue(i, sortPhase, globalHistogramHigh, start, end, false);
    }

    // Wait for the second pass to complete
    threadPool.await();

    // Final swap to restore sorted order in the primary array
    std::swap(map.tileMap, map.tmpTile);
    std::swap(map.idMap, map.tmpId);
}

void init(ThreadMap& map, const int count) {
    map.idMap.resize(count);
    map.tileMap.resize(count);
    map.tmpTile.resize(count);
    map.tmpId.resize(count);
}

void rebuild(ThreadMap& map, ThreadPool& pool, Positions& pos) {
    // build entityToTile
    int startEntity, endEntity;
    for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
        startEntity = ThreadSettings::ENTITIES_PER_THREAD * thread;
        endEntity = startEntity + ThreadSettings::ENTITIES_PER_THREAD;

        pool.enqueue(thread, [&pos, &tileMap = map.tileMap, startEntity, endEntity, thread] {
            __m256 tileWidthVec = _mm256_set1_ps(WorldSettings::TILE_WIDTH);
            __m256 tileHeightVec = _mm256_set1_ps(WorldSettings::TILE_HEIGHT);
            __m256i worldColumnsVec = _mm256_set1_epi32(WorldSettings::WORLD_COLUMNS);
            __m256 invTileWidthVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_WIDTH);
            __m256 invTileHeightVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_HEIGHT);

            __m256 xPosVec;
            __m256 yPosVec;
            __m256 xMul;
            __m256 yMul;
            __m256i tileColVec;
            __m256i tileRowVec;
            __m256i tileRowMul;
            __m256i tileIndexVec;
            __m256i tileIndexVec1;
            __m256i tileIndexVec2;
            __m256i packedIndices;

            int entity = startEntity;
            for (; entity < endEntity - 16; entity += 16) {
                // Prefetch next batch
                _mm_prefetch((const char*)&pos.x[entity + 32], _MM_HINT_T0);
                _mm_prefetch((const char*)&pos.y[entity + 32], _MM_HINT_T0);

                // Process first 8 elements
                xPosVec = _mm256_loadu_ps(&pos.x[entity]);
                yPosVec = _mm256_loadu_ps(&pos.y[entity]);

                xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
                yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);

                tileColVec = _mm256_cvttps_epi32(xMul);
                tileRowVec = _mm256_cvttps_epi32(yMul);

                tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
                tileIndexVec1 = _mm256_add_epi32(tileRowMul, tileColVec);

                // Process next 8 elements
                xPosVec = _mm256_loadu_ps(&pos.x[entity + 8]);
                yPosVec = _mm256_loadu_ps(&pos.y[entity + 8]);

                xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
                yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);

                tileColVec = _mm256_cvttps_epi32(xMul);
                tileRowVec = _mm256_cvttps_epi32(yMul);

                tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
                tileIndexVec2 = _mm256_add_epi32(tileRowMul, tileColVec);

                // Pack 32-bit integers to 16-bit integers
                packedIndices = _mm256_packs_epi32(tileIndexVec1, tileIndexVec2);

                // Store packed 16-bit integers
                _mm256_storeu_si256((__m256i*)&tileMap[entity], packedIndices);
            }

            // reset id map
            //  Handle remaining elements
            for (; entity < endEntity; ++entity) {
                float x = pos.x[entity];
                float y = pos.y[entity];

                int tileCol = static_cast<int>(x / WorldSettings::TILE_WIDTH);
                int tileRow = static_cast<int>(y / WorldSettings::TILE_HEIGHT);

                tileMap[entity] = static_cast<uint16_t>(tileRow * WorldSettings::WORLD_COLUMNS + tileCol);
            }
        });

        pool.enqueue(thread, [&idMap = map.idMap, startEntity, endEntity] {
            __m256i sequence = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
            __m256i increment = _mm256_set1_epi32(8);
            __m256i startOffset = _mm256_set1_epi32(startEntity);

            sequence = _mm256_add_epi32(sequence, startOffset);

            int entity = startEntity;
            for (; entity < endEntity - 8; entity += 8) {
                _mm256_storeu_si256((__m256i*)&idMap[entity], sequence);
                sequence = _mm256_add_epi32(sequence, increment);
            }

            for (; entity < endEntity; ++entity) {
                idMap[entity] = entity;
            }
        });
    }

    pool.await();

    radixSort(map, pool);
}

bool isSorted(ThreadMap& map) {
    bool isSorted = true;
    for (int i = 0; i < WorldSettings::ENTITY_COUNT - 1; ++i) {
        Log::info(std::to_string(map.tileMap[i]));
        if (map.tileMap[i] > map.tileMap[i + 1]) {
            isSorted = false;
        }
    }

    return isSorted;
}
