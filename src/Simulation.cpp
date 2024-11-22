#include "Simulation.h"

#include "raylib.h"

#include "CameraMoveSystem.h"
#include "Log.h"
#include "SimulationSettings.h"

Simulation::Simulation() : m_camera() {
    m_camera.offset = {0.f, 0.f};
    m_camera.target = {0.f, 0.f};
    m_camera.rotation = 0.f;
    m_camera.zoom = 1.f;
}

Simulation::~Simulation() {
}

void Simulation::run() {
    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            onResize(GetScreenWidth(), GetScreenHeight());
        }

        gameLoop();
    }
}

bool Simulation::webRun() {
    gameLoop();

    return IsKeyReleased(KEY_ESCAPE);
}

void Simulation::gameLoop() {
    float dt = GetFrameTime();

    handleInput();
    update(dt);
    render();
}

void Simulation::handleInput() {
}

void Simulation::update(float dt) {
    // move camera
    updateCamera(m_camera, dt);

    // set camera rect:
    float offset = 2.f;
    m_cameraRect = getCameraRect();
}

void Simulation::render() const {
    BeginDrawing();
    ClearBackground(GRAY);

    // draw worldTiles
    BeginMode2D(m_camera);
    for (int x = 0; x < WorldSettings::WORLD_COLUMNS; ++x) {
        for (int y = 0; y < WorldSettings::WORLD_ROWS; ++y) {
            DrawRectangleLines(x * WorldSettings::TILE_WIDTH,
                               y * WorldSettings::TILE_HEIGHT,
                               (float)WorldSettings::TILE_WIDTH,
                               (float)WorldSettings::TILE_HEIGHT,
                               BLACK);
        }
    }
    DrawRectangleLinesEx(m_cameraRect, 5.f, YELLOW);
    EndMode2D();

    // draw cameraRect:

    EndDrawing();
}

void Simulation::onResize(int width, int height) {
    Log::info(("New Size: " + std::to_string(width) + " : " + std::to_string(height)));
}

Rectangle Simulation::getCameraRect() const {
    Vector2 cameraPos = m_camera.target;
    Vector2 size = {GetScreenWidth() / m_camera.zoom, GetScreenHeight() / m_camera.zoom};
    return {cameraPos.x, cameraPos.y, size.x, size.y};
}
