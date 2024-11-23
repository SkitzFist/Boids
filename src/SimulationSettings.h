#ifndef BOIDS_SIMULATION_SETTINGS_H
#define BOIDS_SIMULATION_SETTINGS_H

namespace WorldSettings {
inline constexpr const int ENTITY_COUNT = 10000000;

inline constexpr const int WORLD_WIDTH = 512 * 25;
inline constexpr const int WORLD_HEIGHT = 512 * 25;
inline constexpr const int TILE_WIDTH = 512;
inline constexpr const int TILE_HEIGHT = 512;
inline constexpr const int WORLD_COLUMNS = WORLD_WIDTH / TILE_WIDTH;
inline constexpr const int WORLD_ROWS = WORLD_HEIGHT / TILE_HEIGHT;
inline constexpr const int TILE_COUNT = WORLD_ROWS * WORLD_COLUMNS;

} // namespace WorldSettings

#endif