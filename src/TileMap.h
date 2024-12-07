#ifndef BOIDS_TILE_MAP_H
#define BOIDS_TILE_MAP_H

#include <array>
#include <cmath>
#include <immintrin.h>
#include <memory>
#include <mutex>
#include <vector>

#include "Log.h"
#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"
#include "ThreadVector.h"

using Tiles = std::vector<std::vector<int>>;

struct TileMap {
    Tiles map;
    std::vector<int> entityToTile;
    std::vector<Tiles> threadMaps;

    TileMap(int numThreads) {
        map.resize(WorldSettings::TILE_COUNT);
        entityToTile.resize(WorldSettings::ENTITY_COUNT);
        threadMaps.resize(numThreads);
        for (int i = 0; i < numThreads; ++i) {
            threadMaps[i].resize(WorldSettings::TILE_COUNT);
        }
    }

    void clear(ThreadPool& threadPool) {
        int numThreads = ThreadSettings::THREAD_COUNT;
        int numTiles = WorldSettings::TILE_COUNT;

        int tilesPerMap = numTiles / numThreads;
        int remainder = numTiles % numThreads;

        for (int thread = 0; thread < numThreads; ++thread) {
            int startIndex = thread * tilesPerMap;
            int endIndex = startIndex + tilesPerMap;

            if (thread == numThreads - 1) {
                endIndex += remainder - 1;
            }

            // clear tiles
            threadPool.enqueue(thread, [startIndex, endIndex, &map = this->map]() {
                for (int i = startIndex; i < endIndex; ++i) {
                    map[i].clear();
                }
            });
        }
        for (int thread = 0; thread < numThreads; ++thread) {
            threadPool.enqueue(thread, [thread, &threadTiles = this->threadMaps] {
                for (int tile = 0; tile < WorldSettings::TILE_COUNT; ++tile) {
                    threadTiles[thread][tile].clear();
                }
            });
        }

        // threadPool.awaitCompletion();
    }

    void doEntityToTile(ThreadPool& threadPool, Positions& positions) {
        int entitiesPerThread = WorldSettings::ENTITY_COUNT / ThreadSettings::THREAD_COUNT;
        int remainder = WorldSettings::ENTITY_COUNT % ThreadSettings::THREAD_COUNT;

        for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
            int startindex = thread * entitiesPerThread;
            int endIndex = thread + entitiesPerThread;

            if (thread == ThreadSettings::THREAD_COUNT - 1) {
                endIndex += remainder - 1;
            }

            threadPool.enqueue(thread, [&positions = positions, &entityToTile = this->entityToTile, startindex, endIndex]() {
                int tileCol, tileRow, tileIndex;

                for (int i = startindex; i < endIndex; ++i) {
                    tileCol = (int)positions.x[i] / WorldSettings::TILE_WIDTH;
                    tileRow = (int)positions.y[i] / WorldSettings::TILE_HEIGHT;
                    tileIndex = tileRow * WorldSettings::WORLD_COLUMNS + tileCol;
                    entityToTile[i] = tileIndex;
                }
            });
        }

        // threadPool.awaitCompletion();
    }

    void rebuild(ThreadPool& threadPool) {

        int entitiesPerThread = WorldSettings::ENTITY_COUNT / ThreadSettings::THREAD_COUNT;
        int remainders = (WorldSettings::ENTITY_COUNT % ThreadSettings::THREAD_COUNT);

        for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
            int startEntity = thread * entitiesPerThread;
            int endEntity = startEntity + entitiesPerThread;
            if (thread == ThreadSettings::THREAD_COUNT - 1) {
                endEntity += remainders - 1;
            }

            threadPool.enqueue(thread, [startEntity, endEntity, &threadMap = this->threadMaps[thread], &entityToTile = this->entityToTile] {
                int tile = -1;
                for (int entity = startEntity; entity < endEntity; ++entity) {
                    tile = entityToTile[entity];
                    threadMap[tile].emplace_back(entity);
                }
            });
        }

        // // insert into map
        int tilesPerThread = WorldSettings::TILE_COUNT / ThreadSettings::THREAD_COUNT;
        remainders = WorldSettings::TILE_COUNT % ThreadSettings::THREAD_COUNT;

        for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
            int startTile = thread * tilesPerThread;
            int endTile = startTile + tilesPerThread;
            if (thread == ThreadSettings::THREAD_COUNT - 1) {
                endTile += remainders - 1;
            }

            threadPool.enqueue(thread, [startTile, endTile, &threadTiles = this->threadMaps, &tileMap = this->map]() {
                for (int threadTile = 0; threadTile < threadTiles.size(); ++threadTile) {
                    for (int tile = startTile; tile < endTile; ++tile) {
                        tileMap[tile].insert(tileMap[tile].end(), threadTiles[threadTile][tile].begin(), threadTiles[threadTile][tile].end());
                    }
                }
            });
        }

        // threadPool.awaitCompletion();
    }
};

#endif