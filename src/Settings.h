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

namespace ThreadSettings {
constexpr const int THREAD_COUNT = 5;

constexpr const int ENTITIES_PER_THREAD = WorldSettings::ENTITY_COUNT / THREAD_COUNT;
constexpr const int ENTITIES_REMAINDER = WorldSettings::ENTITY_COUNT % THREAD_COUNT;

constexpr const int TILES_PER_THREAD = WorldSettings::TILE_COUNT / THREAD_COUNT;
constexpr const int TILES_REMAINDER = WorldSettings::TILE_COUNT % THREAD_COUNT;
} // namespace ThreadSettings
#endif