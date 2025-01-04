#ifndef BOIDS_TILE_MAP_SIMD_H
#define BOIDS_TILE_MAP_SIMD_H

#include <algorithm>
#include <array>
#include <vector>

#include "Log.h"
#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"

struct TileMap {
    std::vector<uint16_t> tiles;

    // Search

    std::vector<int> tileIndexes;
    // each thread
    std::array<std::vector<int>, ThreadSettings::THREAD_COUNT> threadMap;
};

// build
void rebuild(TileMap& tileMap, ThreadPool& pool, const Positions& pos);

// sorting
void sort(TileMap& tileMap, ThreadPool& pool);

// searches
void search(TileMap& map, ThreadPool& pool, const int tile, std::vector<int>& result);
void search(TileMap& map, ThreadPool& pool, const Rectangle& area, std::vector<int>& result);
void clearSearchMap(TileMap& tileMap, ThreadPool& pool);

void init(TileMap& tileMap, int entityCount, ThreadPool& pool, const Positions& pos);

#endif // BOIDS_TILE_MAP_SIMD_H
