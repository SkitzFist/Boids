#ifndef BOIDS_TILE_MAP_BUFFER_H
#define BOIDS_TILE_MAP_BUFFER_H

#include <array>
#include <vector>

#include "raylib.h"

#include "Positions.h"
#include "ThreadPool.h"
#include "WorldSettings.h"

struct TileMapBuffer {
    std::vector<int> tiles;
    std::vector<int> entityIds;
    std::vector<int> tileStartindex;
    std::vector<int> tilesEntityCount;
};

struct TileMap {
    bool rebuildBuffer;
    std::array<TileMapBuffer, 2> buffers;
};

void init(TileMap& map, const int entityCount, const int tileCount) noexcept;

// rebuilding
void rebuild(TileMap& map,
             ThreadPool& pool,
             const WorldSettings& worldSettings,
             const ThreadSettings& threadSettings,
             const Positions& positions);

void rebuildBuffer(TileMapBuffer& buffer,
                   const WorldSettings& worldSettings,
                   const Positions& positions);

void countSort(TileMapBuffer& buffer,
               uint16_t maxValue);

// Search
void search(TileMapBuffer& buffer,
            const Rectangle& area,
            std::vector<int>& result,
            const WorldSettings& worldSettings,
            const Positions& positions);

#endif
