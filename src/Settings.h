#ifndef BOIDS_SIMULATION_SETTINGS_H
#define BOIDS_SIMULATION_SETTINGS_H

namespace WorldSettingsNameSpace {

inline constexpr const int ENTITY_COUNT = 1000000;

inline constexpr const int WORLD_COLUMNS = 100;
inline constexpr const int WORLD_ROWS = 100;
inline constexpr const int TILE_COUNT = WORLD_ROWS * WORLD_COLUMNS;
inline constexpr const int TILE_WIDTH = 1024;
inline constexpr const int TILE_HEIGHT = 1024;
inline constexpr const int WORLD_WIDTH = TILE_WIDTH * WORLD_COLUMNS;
inline constexpr const int WORLD_HEIGHT = TILE_HEIGHT * WORLD_ROWS;

} // namespace WorldSettingsNameSpace

namespace ThreadSettingsNameSpace {

inline constexpr const int THREAD_COUNT = 10;

inline constexpr const int ENTITIES_PER_THREAD = WorldSettingsNameSpace::ENTITY_COUNT / THREAD_COUNT;
inline constexpr const int TILES_PER_THREAD = WorldSettingsNameSpace::TILE_COUNT / THREAD_COUNT;

} // namespace ThreadSettingsNameSpace
#endif