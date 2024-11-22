#ifndef BOIDS_TILE_MAP_H
#define BOIDS_TILE_MAP_H

#include <array>
#include <vector>

#include "SimulationSettings.h"

struct TileMap {
    std::array<std::vector<int>, WorldSettings::WORLD_COLUMNS * WorldSettings::WORLD_ROWS> map;
};

#endif
