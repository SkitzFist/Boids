#ifndef BOIDS_MORTON_CODE_MAP_H
#define BOIDS_MORTON_CODE_MAP_H

#include <execution>
#include <stdint.h>
#include <vector>

#include "MortonCode.h"
#include "Positions.h"
#include "Settings.h"
#include "ThreadPool.h"

struct MortonMap {
    std::vector<uint32_t> entries;
    std::vector<uint32_t> entriesCpy;
    std::vector<uint32_t*> entriesPtr;
    std::vector<int> ids;
};

void init(MortonMap& map, const int entityCount);

void encode(MortonMap& map, ThreadPool& pool, const PositionsI& pos);

bool needsSorting(std::vector<uint32_t*>& entries, ThreadPool& pool);
void sort(std::vector<uint32_t*>& entries, std::vector<int>& ids);
void sort(std::vector<uint32_t>& entries, std::vector<int>& ids);

void search(MortonMap& map, const MortonRect& rect, std::vector<int>& result, const PositionsI& pos);

#endif
