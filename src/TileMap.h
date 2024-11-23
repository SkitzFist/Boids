#ifndef BOIDS_TILE_MAP_H
#define BOIDS_TILE_MAP_H

#include <vector>

#include "Log.h"
#include "SimulationSettings.h"
#include "ThreadPool.h"

struct Result {
    int mapIndex, entityId;
};

inline void update(int fromIndex, int toIndex, std::vector<int>& entityToMap, std::vector<Result>& results) {
}

inline void clearMap(int startIndex, int endIndex, std::vector<std::vector<int>>& map) {
    for (int i = startIndex; i < endIndex; ++i) {
        map[i].clear();
    }
}

inline void cler(std::vector<int>& tile) {
    tile.clear();
}

struct TileMap {
    std::vector<std::vector<int>> map;
    std::vector<int> entityToMap;

    TileMap() {
        map.resize(WorldSettings::TILE_COUNT);
        entityToMap.resize(WorldSettings::ENTITY_COUNT);
    }

    void update(ThreadPool& threadPool) {
        return;
        int numThreads = threadPool.numThreads;
        int numTiles = static_cast<int>(map.size());

        if (numThreads == 0) {
            numThreads = 1;
        }

        int tilesPerMap = numTiles / numThreads;
        int remainder = numTiles % numThreads;

        std::atomic<int> tasksRemaining(0);

        for (int i = 0; i < numThreads; ++i) {
            int startIndex = i * tilesPerMap;
            int endIndex = startIndex + tilesPerMap;

            if (i == numThreads - 1) {
                endIndex += remainder - 1;
            }

            tasksRemaining.fetch_add(1, std::memory_order_relaxed);

            threadPool.enqueue([startIndex, endIndex, &map = this->map, &tasksRemaining]() {
                clearMap(startIndex, endIndex, map);
                tasksRemaining.fetch_sub(1, std::memory_order_relaxed);
            });
        }

        // Wait for all tasks to complete
        while (tasksRemaining.load(std::memory_order_relaxed) > 0) {
            std::this_thread::yield();
        }
    }

    // void update(ThreadPool& threadPool) {
    //     // clear map
    //     for (std::size_t i = 0; i < map.size(); ++i) {
    //         map[i].clear();
    //     }

    //     // re-add entitites
    //     constexpr size_t simdWidth = 4;
    //     size_t i = 0;
    //     for (; i + simdWidth <= entityToMap.size(); i += simdWidth) {
    //         __m128i indices = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&entityToMap[i]));

    //         alignas(16) int indexArray[simdWidth];
    //         _mm_store_si128(reinterpret_cast<__m128i*>(indexArray), indices);

    //         for (int j = 0; j < simdWidth; ++j) {
    //             map[indexArray[j]].emplace_back(i + j);
    //         }
    //     }

    //     // handle remainders
    //     for (; i < entityToMap.size(); ++i) {
    //         int index = entityToMap[i];
    //         map[index].emplace_back(i);
    //     }
    // }
};

#endif