#ifndef BOIDS_WORLD_SETTINGS_H
#define BOIDS_WORLD_SETTINGS_H

struct WorldSettings {
    int entityCount;

    int columns;
    int rows;
    int tileCount;

    int tileWidth;
    int tileHeight;

    int worldWidth;
    int worldHeight;
};

inline void init(WorldSettings& worldSettings, const int entityCount, const int columns, const int rows, const int tileSize) {
    worldSettings.entityCount = entityCount;
    worldSettings.columns = columns;
    worldSettings.rows = rows;
    worldSettings.tileCount = columns * rows;
    worldSettings.tileWidth = tileSize;
    worldSettings.tileHeight = tileSize;
    worldSettings.worldWidth = tileSize * columns;
    worldSettings.worldHeight = tileSize * rows;
}

#endif