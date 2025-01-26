#ifndef BOIDS_SINGLE_LIST_MAP_H
#define BOIDS_SINGLE_LIST_MAP_H

#if defined(EMSCRIPTEN)
#include <smmintrin.h>
#include <xmmintrin.h>
#endif

#include <cstring>
#include <immintrin.h>
#include <vector>

#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"

#include <algorithm>
#include <execution>

#include "Log.h"

struct SingleListMap {
    std::vector<int> entityToTile;
    std::vector<int> entityToTileCopy;
    std::vector<int> tileToEntity;
    std::vector<int> nrOfEntitiesAtTile;
    std::vector<int> tileStartIndex;

    std::vector<std::vector<int>> localCounts;

    SingleListMap() {
        entityToTile.resize(WorldSettingsNameSpace::ENTITY_COUNT);
        entityToTileCopy.resize(WorldSettingsNameSpace::ENTITY_COUNT);
        tileToEntity.resize(WorldSettingsNameSpace::ENTITY_COUNT);
        nrOfEntitiesAtTile.resize(WorldSettingsNameSpace::TILE_COUNT);
        tileStartIndex.resize(WorldSettingsNameSpace::TILE_COUNT);

        localCounts.resize(ThreadSettingsNameSpace::THREAD_COUNT);
        for (int i = 0; i < localCounts.size(); ++i) {
            localCounts[i].resize(WorldSettingsNameSpace::TILE_COUNT);
        }
    }

#if defined(EMSCRIPTEN)
    void rebuild(ThreadPool& pool, const Positions& pos) {
        int startEntity, endEntity;
        for (int thread = 0; thread < ThreadSettingsNameSpace::THREAD_COUNT; ++thread) {
            startEntity = ThreadSettingsNameSpace::ENTITIES_PER_THREAD * thread;
            endEntity = startEntity + ThreadSettingsNameSpace::ENTITIES_PER_THREAD;

            // if (thread == ThreadSettingsNameSpace::THREAD_COUNT - 1) {
            //     endEntity += ThreadSettingsNameSpace::ENTITIES_REMAINDER - 1;
            // }

            pool.enqueue(thread, [&pos, &entityToTile = this->entityToTile, startEntity, endEntity, thread] {
                __m128 tileWidthVec = _mm_set_ps1(WorldSettingsNameSpace::TILE_WIDTH);
                __m128 tileHeightVec = _mm_set_ps1(WorldSettingsNameSpace::TILE_HEIGHT);
                __m128i worldColumnsVec = _mm_set1_epi32(WorldSettingsNameSpace::WORLD_COLUMNS);
                __m128 invTileWidthVec = _mm_set_ps1(1.0f / WorldSettingsNameSpace::TILE_WIDTH);
                __m128 invTileHeightVec = _mm_set_ps1(1.0 / WorldSettingsNameSpace::TILE_HEIGHT);
                int entity = startEntity;

                for (; entity < endEntity - 4; entity += 4) {
                    _mm_prefetch((const char*)&pos.x[entity + 8], _MM_HINT_T0);
                    _mm_prefetch((const char*)&pos.y[entity + 8], _MM_HINT_T0);

                    __m128 xPosVec = _mm_loadu_ps(&pos.x[entity]);
                    __m128 yPosVec = _mm_loadu_ps(&pos.y[entity]);

                    __m128 xMul = _mm_mul_ps(xPosVec, invTileWidthVec);
                    __m128 yMul = _mm_mul_ps(yPosVec, invTileHeightVec);

                    __m128i tileColVec = _mm_cvttps_epi32(xMul);
                    __m128i tileRowVec = _mm_cvttps_epi32(yMul);

                    __m128i tileRowMul = _mm_mullo_epi32(tileRowVec, worldColumnsVec);
                    __m128i tileIndexVec = _mm_add_epi32(tileRowMul, tileColVec);

                    _mm_storeu_si128((__m128i*)&entityToTile[entity], tileIndexVec);
                }
            });
        }

        pool.await();
    }

#else

    void rebuild(ThreadPool& pool, const Positions& pos) {
        // build entityToTile
        int startEntity, endEntity;
        for (int thread = 0; thread < ThreadSettingsNameSpace::THREAD_COUNT; ++thread) {
            startEntity = ThreadSettingsNameSpace::ENTITIES_PER_THREAD * thread;
            endEntity = startEntity + ThreadSettingsNameSpace::ENTITIES_PER_THREAD;

            pool.enqueue(thread, [&pos, &entityToTile = this->entityToTile, &localCounts = this->localCounts[thread], startEntity, endEntity, thread] {
                __m256 tileWidthVec = _mm256_set1_ps(WorldSettingsNameSpace::TILE_WIDTH);
                __m256 tileHeightVec = _mm256_set1_ps(WorldSettingsNameSpace::TILE_HEIGHT);
                __m256i worldColumnsVec = _mm256_set1_epi32(WorldSettingsNameSpace::WORLD_COLUMNS);
                __m256 invTileWidthVec = _mm256_set1_ps(1.0f / WorldSettingsNameSpace::TILE_WIDTH);
                __m256 invTileHeightVec = _mm256_set1_ps(1.0f / WorldSettingsNameSpace::TILE_HEIGHT);

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

        // // clean local count on main thread while threads build entityToTile
        // for (int thread = 0; thread < ThreadSettingsNameSpace::THREAD_COUNT; ++thread) {
        //     memset(localCounts[thread].data(), 0, WorldSettingsNameSpace::TILE_COUNT * sizeof(int));
        // }

        pool.await();

        // memcpy(entityToTileCopy.data(), entityToTile.data(), sizeof(int) * WorldSettingsNameSpace::ENTITY_COUNT);

        // std::sort(std::execution::par, entityToTileCopy.begin(), entityToTileCopy.end());

        // // set entity count per tile
        // for (int thread = 0; thread < ThreadSettingsNameSpace::THREAD_COUNT; ++thread) {
        //     startEntity = ThreadSettingsNameSpace::ENTITIES_PER_THREAD * thread;
        //     endEntity = startEntity + ThreadSettingsNameSpace::ENTITIES_PER_THREAD;
        //     pool.enqueue(thread, [&localCount = this->localCounts[thread], &entityToTile = this->entityToTile, startEntity, endEntity] {
        //         int tileIndex;
        //         for (int entity = startEntity; entity < endEntity; ++entity) {
        //             tileIndex = entityToTile[entity];
        //             ++localCount[tileIndex];
        //         }
        //     });
        // }

        pool.await();
    }

#endif
};

#endif
