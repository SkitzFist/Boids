#ifndef BOIDS_SIMULATION_SETTINGS_H
#define BOIDS_SIMULATION_SETTINGS_H

namespace WorldSettings {

inline constexpr const int WORLD_WIDTH = 1920;
inline constexpr const int WORLD_HEIGHT = 1080;
inline constexpr const int TILE_WIDTH = 64;
inline constexpr const int TILE_HEIGHT = 64;
inline constexpr const int WORLD_COLUMNS = WORLD_WIDTH / TILE_WIDTH;
inline constexpr const int WORLD_ROWS = WORLD_HEIGHT / TILE_HEIGHT;

} // namespace WorldSettings

#endif