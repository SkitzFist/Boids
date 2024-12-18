#ifndef BOIDS_SIMULATION_H
#define BOIDS_SIMULATION_H

#include "raylib.h"

#include "Positions.h"
#include "SingleListMap.h"
#include "ThreadPool.h"
#include "TileMap.h"
#include "TileMapSimd.h"

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
    // TileMap m_tileMap;
    // TileMapSIMD m_tileMapSimd;
    SingleListMap m_singleListMap;

    // Components
    Positions m_positions;
    PositionsI m_positionsI;

    // textures
    Texture2D m_circleTexture;

  private:
    void gameLoop();
    void handleInput();
    void update(float dt);
    void render() const;
};

#endif
