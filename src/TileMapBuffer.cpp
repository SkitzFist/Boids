#include "TileMapBuffer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

// debug
#include "Log.h"
#include <iostream>

void init(TileMap& map, const int entityCount, const int tileCount) noexcept {
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
    //

    pool.awaitTileMap();
    map.rebuildBuffer = !map.rebuildBuffer;

    pool.enqueue(threadSettings.tileMapThread,
                 rebuildBuffer,
                 std::ref(map.buffers[map.rebuildBuffer]),
                 std::ref(worldSettings),
                 std::ref(positions));

    // reset entity ids so that index 0 represents entityId 0 etc..
    pool.enqueue(threadSettings.tileMapThread, [worldSettings, &buffer = map.buffers[map.rebuildBuffer]] {
        __m256i sequence = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
        __m256i increment = _mm256_set1_epi32(8);

        int entity = 0;
        for (; entity < worldSettings.entityCount - 8; entity += 8) {
            _mm256_storeu_si256((__m256i*)&buffer.entityIds[entity], sequence);
            sequence = _mm256_add_epi32(sequence, increment);
        }

        for (; entity < worldSettings.entityCount; ++entity) {
            buffer.entityIds[entity] = entity;
        }
    });

    pool.enqueue(threadSettings.tileMapThread,
                 countSort,
                 std::ref(map.buffers[map.rebuildBuffer]),
                 worldSettings.tileCount);
}

#if defined(EMSCRIPTEN)
void rebuildBuffer(TileMapBuffer& buffer, const WorldSettings& WorldSettings, const Positions& Positions) {
}

#else

void rebuildBuffer(TileMapBuffer& buffer, const WorldSettings& worldSettings, const Positions& pos) {
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

    int entity = 0;
    // todo align vector
    for (; entity < worldSettings.entityCount - 8; entity += 8) {
        // Prefetch next batch
        _mm_prefetch((const char*)&pos.x[entity + 16], _MM_HINT_T0);
        _mm_prefetch((const char*)&pos.y[entity + 16], _MM_HINT_T0);

        xPosVec = _mm256_loadu_ps(&pos.x[entity]);
        yPosVec = _mm256_loadu_ps(&pos.y[entity]);

        xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
        yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);

        tileColVec = _mm256_cvttps_epi32(xMul);
        tileRowVec = _mm256_cvttps_epi32(yMul);

        tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
        tileIndexVec = _mm256_add_epi32(tileRowMul, tileColVec);

        _mm256_storeu_si256((__m256i*)&buffer.tiles[entity], tileIndexVec);
    }

    // reset id map
    //  Handle remaining elements
    for (; entity < worldSettings.entityCount; ++entity) {
        float x = pos.x[entity];
        float y = pos.y[entity];

        int tileCol = static_cast<int>(x / worldSettings.tileWidth);
        int tileRow = static_cast<int>(y / worldSettings.tileHeight);

        buffer.tiles[entity] = static_cast<uint16_t>(tileRow * worldSettings.columns + tileCol);
    }
}

#endif

void countSort(TileMapBuffer& buffer,
               uint16_t maxValue) {

    std::fill(buffer.tilesEntityCount.begin(), buffer.tilesEntityCount.end(), 0);
    std::fill(buffer.tileStartindex.begin(), buffer.tileStartindex.end(), -1);

    size_t n = buffer.tiles.size();
    // 1. Count frequencies of each tile value
    std::vector<size_t> count(maxValue + 1, 0);
    for (size_t i = 0; i < n; ++i) {
        ++count[buffer.tiles[i]];
        ++buffer.tilesEntityCount[buffer.tiles[i]];
    }

    // 2. Convert counts to prefix sums so that count[val] becomes
    //    the *ending index (1-based)* of 'val' in the sorted array
    for (size_t val = 1; val <= maxValue; ++val) {
        count[val] += count[val - 1];
    }

    // 3. Allocate temporary arrays for sorted results
    std::vector<int> sortedTiles(n);
    std::vector<int> sortedIds(n);

    // 4. Iterate from the end of the original arrays to place elements
    //    in stable order
    for (size_t i = n; i > 0; --i) {
        uint16_t val = buffer.tiles[i - 1];
        // Decrement count[val] to get the 0-based index where this item goes
        size_t pos = --count[val];
        sortedTiles[pos] = val;
        sortedIds[pos] = buffer.entityIds[i - 1];
    }

    // 5. Move sorted results back into the original vectors
    buffer.tiles.swap(sortedTiles);
    buffer.entityIds.swap(sortedIds);

    // 6. set tileStartIndex
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
            // Row-major index
            const int tileIndex = row * worldSettings.columns + col;

            int entityCount = buffer.tilesEntityCount[tileIndex];
            int startIndex = buffer.tileStartindex[tileIndex];

            if (startIndex < 0 || entityCount <= 0) {
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
