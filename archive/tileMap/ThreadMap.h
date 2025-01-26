#ifndef BOIDS_THREAD_MAP_H
#define BOIDS_THREAD_MAP_H

#include <array>
#include <vector>

#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"

struct ThreadMap {
    std::vector<int> idMap;
    std::vector<int> tmpId;

    std::vector<uint16_t> tileMap;
    std::vector<uint16_t> tmpTile;
};

void init(ThreadMap& map, const int count);

// rebuild
void rebuild(ThreadMap& map, ThreadPool& pool, Positions& pos);
void sort(ThreadMap& map, ThreadPool& pool);
bool isSorted(ThreadMap& map);

#endif