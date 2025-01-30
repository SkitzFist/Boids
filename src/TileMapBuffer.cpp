#include "TileMapBuffer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "AlignedAllocator.h"

// debug
#include "Log.h"
#include <Timer.h>

void init(TileMap& map, const int entityCount, const int tileCount) noexcept {
    map.rebuildBuffer = true;
    for (int i = 0; i < 2; ++i) {
        map.buffers[i].entityIds.resize(entityCount);
        map.buffers[i].entityToTile.resize(entityCount);
        map.buffers[i].tileStartindex.resize(tileCount);
        map.buffers[i].tilesEntityCount.resize(tileCount);
    }
}

void rebuild(TileMap& map,
             ThreadPool& pool,
             const WorldSettings& worldSettings,
             const ThreadSettings& threadSettings,
             const Positions& positions) {
    static Timer t;
    t.begin();

    map.rebuildBuffer = !map.rebuildBuffer;

    int threads = threadSettings.threadCount >> 1;
    int entitiesPerThread = worldSettings.entityCount / threads;

    for (int i = 0; i < threads; ++i) {
        int startEntity = i * entitiesPerThread;
        int endEntity = startEntity + entitiesPerThread;
        // use lower half of threadpool
        pool.enqueue(i,
                     rebuildBuffer,
                     std::ref(map.buffers[map.rebuildBuffer].entityToTile),
                     std::ref(worldSettings),
                     std::ref(positions),
                     startEntity,
                     endEntity);

        // use upper half of threadpool
        int thread = threads + i;
        pool.enqueue(thread,
                     resetEntityIds,
                     std::ref(map.buffers[map.rebuildBuffer].entityIds),
                     startEntity,
                     endEntity);
    }
    // use main core
    std::fill(map.buffers[map.rebuildBuffer].tileStartindex.begin(), map.buffers[map.rebuildBuffer].tileStartindex.end(), -1);
    std::fill(map.buffers[map.rebuildBuffer].tilesEntityCount.begin(), map.buffers[map.rebuildBuffer].tilesEntityCount.end(), 0);

    pool.await();

    pool.enqueue(0,
                 count,
                 std::ref(map.buffers[map.rebuildBuffer].entityToTile),
                 std::ref(map.buffers[map.rebuildBuffer].tileStartindex),
                 std::ref(map.buffers[map.rebuildBuffer].tilesEntityCount));

    pool.enqueue(0,
                 setId,
                 std::ref(map.buffers[map.rebuildBuffer]),
                 worldSettings.entityCount);
    // no await, let it rebuild for next frame.
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

void rebuildBuffer(AlignedInt32Vector& entityToTiles,
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

        _mm256_store_si256((__m256i*)&entityToTiles[entity], tileIndexVec);
        // Log::info("\tstore done!");
    }

    // reset id map
    //  Handle remaining elements
    for (; entity < entityEnd; ++entity) {
        float x = pos.x[entity];
        float y = pos.y[entity];

        int tileCol = static_cast<int>(x / worldSettings.tileWidth);
        int tileRow = static_cast<int>(y / worldSettings.tileHeight);

        entityToTiles[entity] = tileRow * worldSettings.columns + tileCol;
    }

    // Log::info("Rebuild done");
}

void resetEntityIds(AlignedInt32Vector& entityIds,
                    const int startEntity,
                    const int endEntity) {
    __m256i sequence = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
    __m256i increment = _mm256_set1_epi32(8);
    __m256i startOffset = _mm256_set1_epi32(startEntity);

    sequence = _mm256_add_epi32(sequence, startOffset);

    int entity = startEntity;
    for (; entity <= endEntity - 32; entity += 32) {
        _mm_prefetch((const char*)&entityIds[entity + 64], _MM_HINT_T0);

        _mm256_store_si256((__m256i*)&entityIds[entity], sequence);
        sequence = _mm256_add_epi32(sequence, increment);

        _mm256_store_si256((__m256i*)&entityIds[entity + 8], sequence);
        sequence = _mm256_add_epi32(sequence, increment);

        _mm256_store_si256((__m256i*)&entityIds[entity + 16], sequence);
        sequence = _mm256_add_epi32(sequence, increment);

        _mm256_store_si256((__m256i*)&entityIds[entity + 32], sequence);
        sequence = _mm256_add_epi32(sequence, increment);
    }

    for (; entity < endEntity; ++entity) {
        entityIds[entity] = entity;
    }
}

#endif

void count(const AlignedInt32Vector& entityToTiles,
           AlignedInt32Vector& tileStartindex,
           AlignedInt32Vector& tilesEntityCount) {
    for (size_t i = 0; i < entityToTiles.size(); ++i) {
        ++tilesEntityCount[entityToTiles[i]];
    }

    size_t currentIndex = 0;
    for (size_t i = 0; i < tilesEntityCount.size(); ++i) {
        if (tilesEntityCount[i] > 0) {
            tileStartindex[i] = currentIndex;
        }
        currentIndex += tilesEntityCount[i];
    }
}

void setId(TileMapBuffer& buffer, int entityCount) {
    AlignedInt32Vector currentTileIndex = buffer.tileStartindex;

    for (int i = 0; i < entityCount; ++i) {
        int tile = buffer.entityToTile[i];
        buffer.entityIds[currentTileIndex[tile]++] = i;
    }
}

/////////////////////////////////
///         Searching        ///
///////////////////////////////

static inline bool IsColliding(const Rectangle rect,
                               const float x,
                               const float y) {
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

void search(TileMapBuffer& buffer,
            const Rectangle& area,
            std::vector<int>& result,
            const WorldSettings& worldSettings,
            const Positions& positions) {

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
