#ifndef BOIDS_SIMULATION_H
#define BOIDS_SIMULATION_H

#include "raylib.h"

// #include "Positions.h"
#include "ThreadPool.h"
#include "TileMap.h"

class Simulation {
  public:
    Simulation();
    ~Simulation();

    void run();
    bool webRun();

    void onResize(int width, int height);

  private:
    // threadpool
    ThreadPool m_threadPool;

    // camera
    Camera2D m_camera;
    Rectangle m_cameraRect;

    // spatial
    TileMap m_tileMap;

    // Components
    // Positions m_positions;

    // textures
    Texture2D m_circleTexture;

  private:
    void gameLoop();
    void handleInput();
    void update(float dt);
    void render() const;
};

#endif
