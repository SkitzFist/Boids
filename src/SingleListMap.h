#ifndef BOIDS_SINGLE_LIST_MAP_H
#define BOIDS_SINGLE_LIST_MAP_H

#if defined(EMSCRIPTEN)
#include <smmintrin.h>
#include <xmmintrin.h>
#endif

#include <cstring>
#include <immintrin.h>
#include <vector>

#include "AlignedAllocator.h"
#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"

struct SingleListMap {
    AlignedInt32Vector entityToTile;
    std::vector<int> tileToEntity;
    std::vector<int> nrOfEntitiesAtTile;
    std::vector<int> tileStartIndex;

    SingleListMap() {
        entityToTile.resize(WorldSettings::ENTITY_COUNT);
        tileToEntity.resize(WorldSettings::ENTITY_COUNT);
        nrOfEntitiesAtTile.resize(WorldSettings::TILE_COUNT);
        tileStartIndex.resize(WorldSettings::TILE_COUNT);
    }

    void rebuild(ThreadPool& pool, const Positions& pos) {

        int startEntity, endEntity;
        for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
            startEntity = ThreadSettings::ENTITIES_PER_THREAD * thread;
            endEntity = startEntity + ThreadSettings::ENTITIES_PER_THREAD;

            if (thread == ThreadSettings::THREAD_COUNT - 1) {
                endEntity += ThreadSettings::ENTITIES_REMAINDER - 1;
            }

            pool.enqueue(thread, [&pos, &entityToTile = this->entityToTile, startEntity, endEntity] {
                int invTileWidth = 1.0f / WorldSettings::TILE_WIDTH;
                int invTileHeight = 1.0f / WorldSettings::TILE_HEIGHT;
                int col, row, index;
                for (int entity = startEntity; entity < endEntity; ++entity) {
                    col = invTileWidth * pos.x[entity];
                    row = invTileWidth * pos.y[entity];
                    index = row * WorldSettings::WORLD_COLUMNS + col;
                    entityToTile[entity] = index;
                }
            });
        }
    }

    void rebuildSimd(ThreadPool& pool, const Positions& pos) {
        int startEntity, endEntity;
        for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
            startEntity = ThreadSettings::ENTITIES_PER_THREAD * thread;
            endEntity = startEntity + ThreadSettings::ENTITIES_PER_THREAD;

            if (thread == ThreadSettings::THREAD_COUNT - 1) {
                endEntity += ThreadSettings::ENTITIES_REMAINDER - 1;
            }

            pool.enqueue(thread, [&pos, &entityToTile = this->entityToTile, startEntity, endEntity, thread] {
                __m256 tileWidthVec = _mm256_set1_ps(WorldSettings::TILE_WIDTH);
                __m256 tileHeightVec = _mm256_set1_ps(WorldSettings::TILE_HEIGHT);
                __m256i worldColumnsVec = _mm256_set1_epi32(WorldSettings::WORLD_COLUMNS);
                __m256 invTileWidthVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_WIDTH);
                __m256 invTileHeightVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_HEIGHT);
                int entity = startEntity;
                for (; entity < endEntity - 8; entity += 8) {
                    // preftch
                    _mm_prefetch((const char*)&pos.x[entity + 16], _MM_HINT_T0);
                    _mm_prefetch((const char*)&pos.y[entity + 16], _MM_HINT_T0);

                    __m256 xPosVec = _mm256_loadu_ps(&pos.x[entity]);
                    __m256 yPosVec = _mm256_loadu_ps(&pos.y[entity]);

                    __m256 xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
                    __m256 yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);

                    __m256i tileColVec = _mm256_cvttps_epi32(xMul);
                    __m256i tileRowVec = _mm256_cvttps_epi32(yMul);

                    __m256i tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
                    __m256i tileIndexVec = _mm256_add_epi32(tileRowMul, tileColVec);

                    _mm256_storeu_si256((__m256i*)&entityToTile[entity], tileIndexVec);
                }
            });
        }
    }

    void rebuild(const Positions& pos) {
#if defined(EMSCRIPTEN)
        // constants
        __m128 tileWidthVec = _mm_set_ps1(WorldSettings::TILE_WIDTH);
        __m128 tileHeightVec = _mm_set_ps1(WorldSettings::TILE_HEIGHT);
        __m128i worldColumnsVec = _mm_set1_epi32(WorldSettings::WORLD_COLUMNS);
        __m128 invTileWidthVec = _mm_set_ps1(1.0f / WorldSettings::TILE_WIDTH);
        __m128 invTileHeightVec = _mm_set_ps1(1.0 / WorldSettings::TILE_HEIGHT);

        for (int i = 0; i < WorldSettings::ENTITY_COUNT - 4; i += 4) {
            _mm_prefetch((const char*)&pos.x[i + 8], _MM_HINT_T0);
            _mm_prefetch((const char*)&pos.y[i + 8], _MM_HINT_T0);

            __m128 xPosVec = _mm_loadu_ps(&pos.x[i]);
            __m128 yPosVec = _mm_loadu_ps(&pos.y[i]);

            __m128 xMul = _mm_mul_ps(xPosVec, invTileWidthVec);
            __m128 yMul = _mm_mul_ps(yPosVec, invTileHeightVec);

            __m128i tileColVec = _mm_cvttps_epi32(xMul);
            __m128i tileRowVec = _mm_cvttps_epi32(yMul);

            __m128i tileRowMul = _mm_mullo_epi32(tileRowVec, worldColumnsVec);
            __m128i tileIndexVec = _mm_add_epi32(tileRowMul, tileColVec);

            _mm_storeu_si128((__m128i*)&entityToTile[i], tileIndexVec);
        }

#else
        // Constants
        __m256 tileWidthVec = _mm256_set1_ps(WorldSettings::TILE_WIDTH);
        __m256 tileHeightVec = _mm256_set1_ps(WorldSettings::TILE_HEIGHT);
        __m256i worldColumnsVec = _mm256_set1_epi32(WorldSettings::WORLD_COLUMNS);
        __m256 invTileWidthVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_WIDTH);
        __m256 invTileHeightVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_HEIGHT);

        // Process entities in chunks of 8
        for (int i = 0; i < WorldSettings::ENTITY_COUNT - 8; i += 8) {
            // preftch
            _mm_prefetch((const char*)&pos.x[i + 16], _MM_HINT_T0);
            _mm_prefetch((const char*)&pos.y[i + 16], _MM_HINT_T0);

            __m256 xPosVec = _mm256_loadu_ps(&pos.x[i]);
            __m256 yPosVec = _mm256_loadu_ps(&pos.y[i]);

            __m256 xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
            __m256 yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);

            __m256i tileColVec = _mm256_cvttps_epi32(xMul);
            __m256i tileRowVec = _mm256_cvttps_epi32(yMul);

            __m256i tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
            __m256i tileIndexVec = _mm256_add_epi32(tileRowMul, tileColVec);

            _mm256_storeu_si256((__m256i*)&entityToTile[i], tileIndexVec);
        }

        int remainders = WorldSettings::ENTITY_COUNT % 8;
        // Process any remaining entities
        for (int i = WorldSettings::ENTITY_COUNT - remainders; i < WorldSettings::ENTITY_COUNT; +i) {
            int tileCol = (int)(pos.x[i] / WorldSettings::TILE_WIDTH);
            int tileRow = (int)(pos.y[i] / WorldSettings::TILE_HEIGHT);
            int tileIndex = tileRow * WorldSettings::WORLD_COLUMNS + tileCol;
            entityToTile[i] = tileIndex;
        }

#endif
    }
};

#endif
