#ifndef BOIDS_SIMULATION_H
#define BOIDS_SIMULATION_H

#include "raylib.h"

#include "ThreadSettings.h"
#include "WorldSettings.h"

#include "ThreadPool.h"

// spatial
#include "TileMapBuffer.h"

// components
#include "Positions.h"
#include "Velocities.h"

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
    WorldSettings& m_worldSettings;
    ThreadSettings& m_threadSettings;

    // threadpool
    ThreadPool& m_threadPool;

    // camera
    Camera2D m_camera;

    // spatial
    TileMap m_tileMap;
    std::vector<int> m_entitiesInRange;
    Rectangle m_cameraRect;
    std::vector<int> m_entitiesinSearchArea;
    Rectangle m_searchArea;

    // Components
    Positions m_positions;
    Velocities m_velocities;

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
