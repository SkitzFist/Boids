#ifndef BOIDS_SIMULATION_H
#define BOIDS_SIMULATION_H

class Simulation {
  public:
    Simulation();
    ~Simulation();

    void run();
    bool webRun();

    void onResize(int width, int height);

  private:
    void gameLoop();
    void handleInput();
    void update(float dt);
    void render() const;
};

#endif
