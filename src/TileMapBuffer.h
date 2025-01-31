#ifndef BOIDS_TILE_MAP_BUFFER_H
#define BOIDS_TILE_MAP_BUFFER_H

#include <array>
#include <atomic>
#include <vector>

#include "raylib.h"

#include "AlignedAllocator.h"
#include "Positions.h"
#include "ThreadPool.h"
#include "WorldSettings.h"

struct TileMapBuffer {
    AlignedInt32Vector entityToTile;
    AlignedInt32Vector entityIds;
    AlignedInt32Vector tileStartindex;
    AlignedInt32Vector tilesEntityCount;
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

void rebuildJob(TileMapBuffer& buffer,
                ThreadPool& pool,
                const WorldSettings& worldSettings,
                const ThreadSettings& threadSettings,
                const Positions& positions);

void rebuildBuffer(AlignedInt32Vector& entityToTile,
                   const WorldSettings& worldSettings,
                   const Positions& positions,
                   const int entityStart,
                   const int entityEnd);

void resetEntityIds(AlignedInt32Vector& entityIds,
                    const int startEntity,
                    const int endEntity);

void resetTo(AlignedInt32Vector& vec,
             const int value,
             const int entityStart,
             const int entityEnd);

void count(const AlignedInt32Vector& entityToTile,
           AlignedInt32Vector& tileStartindex,
           AlignedInt32Vector& tilesEntityCount);

void setId(TileMapBuffer& buffer, int entityCount);

// Search
void search(TileMapBuffer& buffer,
            const Rectangle& area,
            std::vector<int>& result,
            const WorldSettings& worldSettings,
            const Positions& positions);

#endif
