#include "TileMapBuffer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "AlignedAllocator.h"

// debug
#include "Log.h"

void init(TileMap& map, const int entityCount, const int tileCount) noexcept {
    map.rebuildBuffer = true;
    for (int i = 0; i < 2; ++i) {
        map.buffers[i].entityIds.resize(entityCount);
        map.buffers[i].tiles.resize(entityCount);
        map.buffers[i].tileStartindex.resize(tileCount);
        map.buffers[i].tilesEntityCount.resize(tileCount);
    }
}

void rebuild(TileMap& map,
             ThreadPool& pool,
             const WorldSettings& worldSettings,
             const ThreadSettings& threadSettings,
             const Positions& positions) {
    // pool.await(); // all workers needs to be finished to avoid race conditions.
    // map.rebuildBuffer = !map.rebuildBuffer;

    // // Log::info("Threads: " + std::to_string(threadSettings.workerCount));
    // int threads = 1; // threadSettings.workerCount >> 1; // uses lower half of threadpool
    // int entitiesPerWorker = worldSettings.entityCount / threads;

    // for (int i = 0; i < threads; ++i) {
    //     // Log::info("T: " + std::to_string(i));
    //     int entityStart = entitiesPerWorker * i;
    //     int entityEnd = entityStart + entitiesPerWorker;
    //     pool.enqueue(i,
    //                  rebuildBuffer,
    //                  std::ref(map.buffers[map.rebuildBuffer].tiles),
    //                  std::ref(worldSettings),
    //                  std::ref(positions),
    //                  entityStart,
    //                  entityEnd);
    // }

    // // await rebuildBuffer to avoid race condition when updating Positions.
    // pool.awaitWorkers(0, threads);

    // for (int i = 0; i < threads; ++i) {
    //     int entityStart = entitiesPerWorker * i;
    //     int entityEnd = entityStart + entitiesPerWorker;
    //     pool.enqueue(i,
    //                  resetEntityIds,
    //                  std::ref(worldSettings),
    //                  std::ref(map.buffers[map.rebuildBuffer].entityIds),
    //                  entityStart,
    //                  entityEnd);
    // }

    // pool.enqueue(0,
    //              countSort,
    //              std::ref(map.buffers[map.rebuildBuffer]));

    map.rebuildBuffer = !map.rebuildBuffer;

    pool.enqueue(0,
                 rebuildBuffer,
                 std::ref(map.buffers[map.rebuildBuffer].tiles),
                 std::ref(worldSettings),
                 std::ref(positions),
                 0,
                 worldSettings.entityCount);
    pool.awaitWorkers(0, 1);

    pool.enqueue(0,
                 resetEntityIds,
                 std::ref(worldSettings),
                 std::ref(map.buffers[map.rebuildBuffer].entityIds),
                 0,
                 worldSettings.entityCount);

    pool.enqueue(0,
                 countSort,
                 std::ref(map.buffers[map.rebuildBuffer]));
}

#if defined(EMSCRIPTEN)
void rebuildBuffer(std::vector<int>& tiles,
                   const WorldSettings& worldSettings,
                   const Positions& pos,
                   const int entityStart,
                   const int entityEnd) {
    __m128i worldColumnsVec = _mm_set1_epi32(worldSettings.columns);
    __m128 invTileWidthVec = _mm_set1_ps(1.f / worldSettings.tileWidth);
    __m128 invTileHeightVec = _mm_set1_ps(1.f / worldSettings.tileHeight);
    __m128 xPosVec;
    __m128 yPosVec;
    __m128 xMul;
    __m128 yMul;
    __m128i tileColVec;
    __m128i tileRowVec;
    __m128i tileRowMul;
    __m128i tileIndexVec;

    int entity = entityStart;
    for (; entity <= entityEnd - 4; entity += 4) {
        // Log::info("\tEntity: " + std::to_string(entity));
        _mm_prefetch((const char*)&pos.x[entity + 8], _MM_HINT_T0);
        _mm_prefetch((const char*)&pos.y[entity + 8], _MM_HINT_T0);

        // Log::info("\tPrefecth done!");

        xPosVec = _mm_load_ps(&pos.x[entity]);
        yPosVec = _mm_load_ps(&pos.y[entity]);
        // Log::info("\tLoad done!");

        xMul = _mm_mul_ps(xPosVec, invTileWidthVec);
        yMul = _mm_mul_ps(yPosVec, invTileHeightVec);
        // Log::info("\tMul done!");

        tileColVec = _mm_cvttps_epi32(xMul);
        tileRowVec = _mm_cvttps_epi32(yMul);
        // Log::info("\tCast done!");

        tileRowMul = _mm_mullo_epi32(tileRowVec, worldColumnsVec);
        tileIndexVec = _mm_add_epi32(tileRowMul, tileColVec);

        // Log::info("\tindex done!");

        _mm_storeu_si128((__m128i*)&tiles[entity], tileIndexVec);
        // Log::info("\tstore done!");
    }

    // reset id map
    //  Handle remaining elements
    for (; entity <= entityEnd; ++entity) {
        float x = pos.x[entity];
        float y = pos.y[entity];

        int tileCol = static_cast<int>(x / worldSettings.tileWidth);
        int tileRow = static_cast<int>(y / worldSettings.tileHeight);

        tiles[entity] = tileRow * worldSettings.columns + tileCol;
    }
}

void resetEntityIds(const WorldSettings& worldSettings, std::vector<int>& entityIds) {
    __m128i sequence = _mm_setr_epi32(0, 1, 2, 3);
    __m128i increment = _mm_set1_epi32(4);

    int entity = 0;
    for (; entity < worldSettings.entityCount - 4; entity += 4) {
        _mm_storeu_si128((__m128i*)&entityIds[entity], sequence);
        sequence = _mm_add_epi32(sequence, increment);
    }

    for (; entity < worldSettings.entityCount; ++entity) {
        entityIds[entity] = entity;
    }
}

#else

void rebuildBuffer(std::vector<int>& tiles,
                   const WorldSettings& worldSettings,
                   const Positions& pos,
                   const int entityStart,
                   const int entityEnd) {
    __m256i worldColumnsVec = _mm256_set1_epi32(worldSettings.columns);
    __m256 invTileWidthVec = _mm256_set1_ps(1.0f / worldSettings.tileWidth);
    __m256 invTileHeightVec = _mm256_set1_ps(1.0f / worldSettings.tileHeight);

    __m256 xPosVec;
    __m256 yPosVec;
    __m256 xMul;
    __m256 yMul;
    __m256i tileColVec;
    __m256i tileRowVec;
    __m256i tileRowMul;
    __m256i tileIndexVec;

    int entity = entityStart;
    for (; entity <= entityEnd - 8; entity += 8) {
        // Log::info("\tEntity: " + std::to_string(entity));
        _mm_prefetch((const char*)&pos.x[entity + 16], _MM_HINT_T0);
        _mm_prefetch((const char*)&pos.y[entity + 16], _MM_HINT_T0);

        // Log::info("\tPrefecth done!");

        xPosVec = _mm256_load_ps(&pos.x[entity]);
        yPosVec = _mm256_load_ps(&pos.y[entity]);
        // Log::info("\tLoad done!");

        xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
        yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);
        // Log::info("\tMul done!");

        tileColVec = _mm256_cvttps_epi32(xMul);
        tileRowVec = _mm256_cvttps_epi32(yMul);
        // Log::info("\tCast done!");

        tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
        tileIndexVec = _mm256_add_epi32(tileRowMul, tileColVec);

        // Log::info("\tindex done!");

        _mm256_storeu_si256((__m256i*)&tiles[entity], tileIndexVec);
        // Log::info("\tstore done!");
    }

    // reset id map
    //  Handle remaining elements
    for (; entity < entityEnd; ++entity) {
        float x = pos.x[entity];
        float y = pos.y[entity];

        int tileCol = static_cast<int>(x / worldSettings.tileWidth);
        int tileRow = static_cast<int>(y / worldSettings.tileHeight);

        tiles[entity] = tileRow * worldSettings.columns + tileCol;
    }

    // Log::info("Rebuild done");
}

void resetEntityIds(const WorldSettings& worldSettings,
                    std::vector<int>& entityIds,
                    const int startEntity,
                    const int endEntity) {
    __m256i sequence = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
    __m256i increment = _mm256_set1_epi32(8);
    __m256i startOffset = _mm256_set1_epi32(startEntity);

    sequence = _mm256_add_epi32(sequence, startOffset);

    int entity = startEntity;
    for (; entity <= endEntity - 8; entity += 8) {
        _mm256_storeu_si256((__m256i*)&entityIds[entity], sequence);
        sequence = _mm256_add_epi32(sequence, increment);
    }

    for (; entity < endEntity; ++entity) {
        entityIds[entity] = entity;
    }
}

#endif

/* Leaks memory on closing. Not sure what causes it though.
 Pretty sure it's a thread issue. The TileMapBuffer has gone out of scope.
*/
void countSort(TileMapBuffer& buffer) {

    std::fill(buffer.tilesEntityCount.begin(), buffer.tilesEntityCount.end(), 0);
    std::fill(buffer.tileStartindex.begin(), buffer.tileStartindex.end(), -1);

    size_t n = buffer.tiles.size();
    std::vector<size_t> count(n, 0);
    for (size_t i = 0; i < n; ++i) {
        ++count[buffer.tiles[i]];
        ++buffer.tilesEntityCount[buffer.tiles[i]];
    }

    for (size_t val = 1; val < n; ++val) {
        count[val] += count[val - 1];
    }

    std::vector<int> sortedTiles(n);
    std::vector<int> sortedIds(n);

    for (size_t i = n; i > 0; --i) {
        uint16_t val = buffer.tiles[i - 1];
        size_t pos = --count[val];
        sortedTiles[pos] = val;
        sortedIds[pos] = buffer.entityIds[i - 1];
    }

    buffer.tiles.swap(sortedTiles);
    buffer.entityIds.swap(sortedIds);

    // set tileStartIndex
    size_t currentIndex = 0;
    for (size_t i = 0; i < buffer.tilesEntityCount.size(); ++i) {
        if (buffer.tilesEntityCount[i] > 0) {
            buffer.tileStartindex[i] = currentIndex;
        }
        currentIndex += buffer.tilesEntityCount[i];
    }
}

/////////////////////////////////
///         Searching        ///
///////////////////////////////

static inline bool IsColliding(const Rectangle rect, const float x, const float y) {
    if (x < rect.x)
        return false;

    if (x > rect.x + rect.width)
        return false;

    if (y < rect.y)
        return false;

    if (y > rect.y + rect.height)
        return false;

    return true;
}

void search(TileMapBuffer& buffer, const Rectangle& area, std::vector<int>& result, const WorldSettings& worldSettings, const Positions& positions) {

    int startRow = static_cast<int>(std::floor(area.y / worldSettings.tileHeight));
    int endRow = static_cast<int>(std::ceil((area.y + area.height) / worldSettings.tileHeight));
    int startCol = static_cast<int>(std::floor(area.x / worldSettings.tileWidth));
    int endCol = static_cast<int>(std::ceil((area.x + area.width) / worldSettings.tileWidth));

    if (startRow < 0)
        startRow = 0;
    if (startCol < 0)
        startCol = 0;
    if (endRow > worldSettings.rows)
        endRow = worldSettings.rows;
    if (endCol > worldSettings.columns)
        endCol = worldSettings.columns;

    for (int row = startRow; row < endRow; ++row) {
        for (int col = startCol; col < endCol; ++col) {
            const int tileIndex = row * worldSettings.columns + col;

            int entityCount = buffer.tilesEntityCount[tileIndex];
            int startIndex = buffer.tileStartindex[tileIndex];

            if (startIndex < 0 || entityCount == 0) {
                continue;
            }

            for (int i = 0; i < entityCount; ++i) {
                int entityId = buffer.entityIds[startIndex + i];

                if (IsColliding(area, positions.x[entityId], positions.y[entityId])) {
                    result.emplace_back(entityId);
                }
            }
        }
    }
}
