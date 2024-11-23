#include "Simulation.h"

#include "raylib.h"

#include "CameraMoveSystem.h"
#include "Log.h"
#include "SimulationSettings.h"
#include "ThreadPool.h"

inline void test(int id) {
    Log::info("id: " + std::to_string(id));
}

Simulation::Simulation() : m_threadPool(4), m_camera(), m_tileMap() {
    m_camera.offset = {0.f, 0.f};
    m_camera.target = {0.f, 0.f};
    m_camera.rotation = 0.f;
    m_camera.zoom = 1.f;

    RenderTexture2D renderTexture = LoadRenderTexture(32, 32);
    BeginTextureMode(renderTexture);
    DrawCircle(16, 16, 16, RED);
    EndTextureMode();
    m_circleTexture = renderTexture.texture;
    UnloadRenderTexture(renderTexture);

    for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {
        m_tileMap.entityToMap[i] = GetRandomValue(0, WorldSettings::TILE_COUNT - 1);
    }
}

Simulation::~Simulation() {
    UnloadTexture(m_circleTexture);
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
    m_cameraRect = {m_camera.target.x, m_camera.target.y, GetScreenWidth() / m_camera.zoom, GetScreenHeight() / m_camera.zoom};

    // should update after entities moved
    m_tileMap.update(m_threadPool);
}

void Simulation::render() const {
    BeginDrawing();
    ClearBackground(GRAY);

    // draw worldTiles
    BeginMode2D(m_camera);
    // for (int x = 0; x < WorldSettings::WORLD_COLUMNS; ++x) {
    //     for (int y = 0; y < WorldSettings::WORLD_ROWS; ++y) {
    //         DrawRectangleLines(x * WorldSettings::TILE_WIDTH,
    //                            y * WorldSettings::TILE_HEIGHT,
    //                            (float)WorldSettings::TILE_WIDTH,
    //                            (float)WorldSettings::TILE_HEIGHT,
    //                            BLACK);
    //     }
    // }

    // draw circles

    // draw camera rect
    DrawRectangleLinesEx(m_cameraRect, 5.f, YELLOW);
    EndMode2D();

    // draw UI

    DrawText(("FPS: " + std::to_string(GetFPS())).c_str(), 10, 10, 20, BLACK);
    DrawText(("Entities: " + std::to_string(WorldSettings::ENTITY_COUNT)).c_str(), 10, 30, 20, BLACK);

    EndDrawing();
}

void Simulation::onResize(int width, int height) {
    Log::info(("New Size: " + std::to_string(width) + " : " + std::to_string(height)));
}
