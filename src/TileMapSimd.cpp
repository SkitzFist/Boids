#include "TileMapSimd.h"

#include <cmath>
#include <unordered_set>

void rebuild(TileMap& tileMap, ThreadPool& pool, const Positions& pos) {
    // build entityToTile
    int startEntity, endEntity;
    for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
        startEntity = ThreadSettings::ENTITIES_PER_THREAD * thread;
        endEntity = startEntity + ThreadSettings::ENTITIES_PER_THREAD;

        pool.enqueue(thread, [&pos, &tileMap = tileMap, startEntity, endEntity, thread] {
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
            for (; entity < endEntity - 16; entity += 16) { // Process 16 elements at a time now
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
                _mm256_storeu_si256((__m256i*)&tileMap.tiles[entity], packedIndices);
            }

            // Handle remaining elements
            for (; entity < endEntity; ++entity) {
                float x = pos.x[entity];
                float y = pos.y[entity];

                int tileCol = static_cast<int>(x / WorldSettings::TILE_WIDTH);
                int tileRow = static_cast<int>(y / WorldSettings::TILE_HEIGHT);

                tileMap.tiles[entity] = static_cast<uint16_t>(tileRow * WorldSettings::WORLD_COLUMNS + tileCol);
            }
        });
    }

    pool.await();
}

inline static void bubbleSort(std::vector<uint16_t>& arr) {
    int n = arr.size();
    bool swapped;

    for (int i = 0; i < n - 1; i++) {
        swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        // If no two elements were swapped, then break
        if (!swapped)
            break;
    }
}

void sort(TileMap& map, ThreadPool& pool) {
    bubbleSort(map.tiles);
}

void clearSearchMap(TileMap& tileMap, ThreadPool& pool) {
    pool.enqueue(0, [&threadMap = tileMap.threadMap] {
        for (int i = 0; i < threadMap.size(); ++i) {
            threadMap[i].clear();
        }
    });
}

void search(TileMap& map, ThreadPool& pool, const int tile, std::vector<int>& result) {

    int startEntity, endEntity;

    for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
        startEntity = thread * ThreadSettings::ENTITIES_PER_THREAD;
        endEntity = startEntity + ThreadSettings::ENTITIES_PER_THREAD;

        pool.enqueue(thread, [&tiles = map.tiles, &threadMap = map.threadMap, startEntity, endEntity, tile, thread] {
            for (int i = startEntity; i < endEntity; ++i) {
                if (tiles[i] == tile) {
                    threadMap[thread].emplace_back(i);
                }
            }
        });
    }

    pool.await();

    for (int i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
        for (int j = 0; j < map.threadMap[i].size(); ++j) {
            result.emplace_back(map.threadMap[i][j]);
        }
    }

    clearSearchMap(map, pool);
}

void search(TileMap& map, ThreadPool& pool, const Rectangle& area, std::vector<int>& result) {
    std::size_t estimatedNumTiles =
        (static_cast<std::size_t>(area.height) / WorldSettings::TILE_HEIGHT) *
        (static_cast<std::size_t>(area.width) / WorldSettings::TILE_WIDTH);

    map.tileIndexes.reserve(estimatedNumTiles);

    // Include all tile indexes the rectangle overlaps
    const int startRow = static_cast<int>(std::floor(area.y / WorldSettings::TILE_HEIGHT));
    const int endRow = static_cast<int>(std::ceil((area.y + area.height) / WorldSettings::TILE_HEIGHT));
    const int startCol = static_cast<int>(std::floor(area.x / WorldSettings::TILE_WIDTH));
    const int endCol = static_cast<int>(std::ceil((area.x + area.width) / WorldSettings::TILE_WIDTH));

    for (int row = startRow; row < endRow; ++row) {
        for (int col = startCol; col < endCol; ++col) {
            // Row-major index
            const int tileIndex = row * WorldSettings::WORLD_COLUMNS + col;
            map.tileIndexes.emplace_back(tileIndex);
        }
    }

    std::sort(map.tileIndexes.begin(), map.tileIndexes.end()); // Sort for SIMD-friendly access

    int startEntity, endEntity;
    for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
        startEntity = thread * ThreadSettings::ENTITIES_PER_THREAD;
        endEntity = startEntity + ThreadSettings::ENTITIES_PER_THREAD;

        pool.enqueue(thread, [&map = map, startEntity, endEntity, thread] {
            for (int i = startEntity; i < endEntity; ++i) {
                auto it = std::lower_bound(map.tileIndexes.begin(), map.tileIndexes.end(), map.tiles[i]);
                if (it != map.tileIndexes.end() && *it == map.tiles[i]) {
                    map.threadMap[thread].emplace_back(i);
                }
            }
        });
    }

    pool.await();

    for (int i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
        result.insert(result.end(), map.threadMap[i].begin(), map.threadMap[i].end());
    }

    clearSearchMap(map, pool);
}

void init(TileMap& tileMap, int entityCount, ThreadPool& pool, const Positions& pos) {
    tileMap.tiles.resize(entityCount);

    rebuild(tileMap, pool, pos);

    for (int i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
        tileMap.threadMap[i].reserve(20000);
    }
}