#ifndef BOIDS_THREAD_MAP_H
#define BOIDS_THREAD_MAP_H

#include <array>
#include <vector>

#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"

struct ThreadMap {
    std::vector<int> idMap;
    std::vector<uint16_t> tileMap;
    std::vector<uint16_t> tmpMap;
};

void init(ThreadMap& map);

// rebuild
void rebuild(std::vector<uint16_t>& tileMap, std::vector<int>& idMap, ThreadPool& pool, Positions& pos);
void sort(ThreadMap& map, ThreadPool& pool);

#endif