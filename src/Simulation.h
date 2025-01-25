#ifndef BOIDS_SIMULATION_H
#define BOIDS_SIMULATION_H

#include "raylib.h"

#include "ThreadSettings.h"
#include "WorldSettings.h"

#include "ThreadPool.h"

// spatial
// #include "MortonCodeMap.h"
// #include "SingleListMap.h"
// #include "ThreadMap.h"
// #include "TileMapSimd.h"
#include "TileMapBuffer.h"

// components
#include "Positions.h"

// util
#include "Timer.h"

class Simulation {
  public:
    Simulation(WorldSettings& worldSettings, ThreadSettings& threadSettings, ThreadPool& threadPool);
    ~Simulation();

    void run();
    bool webRun();

    void onResize(int width, int height);

  private:
    // settings
    WorldSettings& worldSettings;
    ThreadSettings& threadSettings;

    // threadpool
    ThreadPool& m_threadPool;

    // camera
    Camera2D m_camera;
    Rectangle m_cameraRect;

    // spatial
    // TileMap m_tileMap;
    // TileMapSIMD m_tileMapSimd;
    TileMap m_tileMap;
    std::vector<int> m_entitiesInRange;
    // MortonMap m_mortonMap;
    // ThreadMap m_threadMap;

    // Components
    Positions m_positions;
    PositionsI m_positionsI;

    // textures
    Texture2D m_circleTexture;

    // Test util
    Timer m_buildTimer;
    Timer m_searchTimer;

  private:
    void gameLoop();
    void handleInput();
    void update(float dt);
    void render() const;
};

#endif
