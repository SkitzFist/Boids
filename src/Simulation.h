#ifndef BOIDS_SIMULATION_H
#define BOIDS_SIMULATION_H

#include "raylib.h"

class Simulation {
  public:
    Simulation();
    ~Simulation();

    void run();
    bool webRun();

    void onResize(int width, int height);

  private:
    Camera2D m_camera;
    Rectangle m_cameraRect;

  private:
    void gameLoop();
    void handleInput();
    void update(float dt);
    void render() const;

    Rectangle getCameraRect() const;
};

#endif
