#include "Simulation.h"

#include <immintrin.h>

#include "raylib.h"

#include "CameraMoveSystem.h"
#include "Settings.h"
#include "ThreadPool.h"

// debug
#include "Log.h"
#include "Timer.h"

Simulation::Simulation(WorldSettings& worldSettings,
                       ThreadSettings& threadSettings,
                       ThreadPool& threadPool) : m_worldSettings(worldSettings),
                                                 m_threadSettings(threadSettings),
                                                 m_threadPool(threadPool),
                                                 m_camera() {
    m_camera.offset = {0.f, 0.f};
    m_camera.target = {0.f, 0.f};
    m_camera.rotation = 0.f;
    m_camera.zoom = 1.f;

    RenderTexture2D renderTexture = LoadRenderTexture(16, 16);
    BeginTextureMode(renderTexture);
    ClearBackground({0, 0, 0, 0});
    DrawCircle(8, 8, 8, WHITE);
    EndTextureMode();
    m_circleTexture = renderTexture.texture;

    init(m_positions, m_worldSettings.entityCount);

    for (int i = 0; i < worldSettings.entityCount; ++i) {
        m_positions.x[i] = GetRandomValue(0, worldSettings.worldWidth - 16);
        m_positions.y[i] = GetRandomValue(0, worldSettings.worldHeight - 16);
    }

    init(m_tileMap, worldSettings.entityCount, worldSettings.tileCount);
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

    // // tmp move pos
    // for (int i = 0; i < worldSettings.entityCount; ++i) {

    //     int speed = i % 2 == 0 ? 1 : -1;
    //     m_positions.x[i] += GetRandomValue(0, speed);

    //     if (m_positions.x[i] > worldSettings.worldWidth) {
    //         m_positions.x[i] = 0;
    //     } else if (m_positions.x[i] < 0) {
    //         m_positions.x[i] = worldSettings.worldWidth - 16;
    //     }
    // }

    // tileMapBuffer
    rebuild(m_tileMap, m_threadPool, m_worldSettings, m_threadSettings, m_positions);

    m_entitiesInRange.clear();
    search(m_tileMap.buffers[!m_tileMap.rebuildBuffer], m_cameraRect, m_entitiesInRange, m_worldSettings, m_positions);

    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), m_camera);
    float size = 200.f;
    m_searchArea = {
        mousePos.x - (size / 2.f), mousePos.y - (size / 2.f),
        size, size};
    m_entitiesinSearchArea.clear();
    search(m_tileMap.buffers[!m_tileMap.rebuildBuffer], m_searchArea, m_entitiesinSearchArea, m_worldSettings, m_positions);
}

void Simulation::render() const {
    BeginDrawing();
    ClearBackground(GRAY);

    // draw worldTiles
    BeginMode2D(m_camera);
    for (int x = 0; x < m_worldSettings.columns; ++x) {
        for (int y = 0; y < m_worldSettings.rows; ++y) {
            DrawRectangleLines(x * m_worldSettings.tileWidth,
                               y * m_worldSettings.tileHeight,
                               (float)m_worldSettings.tileWidth,
                               (float)m_worldSettings.tileHeight,
                               BLACK);
        }
    }

    // draw circles
    Rectangle src = {0.f, 0.f, 16.f, 16.f};
    Rectangle dst = {0.f, 0.f, 16.f, 16.f};

    // for (int i = 0; i < worldSettings.entityCount; ++i) {
    //     dst.x = (float)m_positions.x[i];
    //     dst.y = (float)m_positions.y[i];
    //     DrawTexturePro(m_circleTexture, src, dst, {0.f, 0.f}, 0.f, RED);
    // }

    for (const int i : m_entitiesInRange) {
        dst.x = (float)m_positions.x[i];
        dst.y = (float)m_positions.y[i];
        DrawTexturePro(m_circleTexture, src, dst, {0.f, 0.f}, 0.f, YELLOW);
    }

    for (const int i : m_entitiesinSearchArea) {
        dst.x = (float)m_positions.x[i];
        dst.y = (float)m_positions.y[i];
        DrawTexturePro(m_circleTexture, src, dst, {0.f, 0.f}, 0.f, GREEN);
    }

    DrawRectangleLinesEx(m_searchArea, 1.f, GREEN);

    EndMode2D();

    DrawText(("FPS: " + std::to_string(GetFPS())).c_str(), 10, 10, 20, BLACK);
    DrawText(("Entities: " + std::to_string(m_worldSettings.entityCount)).c_str(), 10, 30, 20, BLACK);
    DrawText(("Drawn: " + std::to_string(m_entitiesInRange.size())).c_str(), 10, 50, 20, BLACK);
    DrawText(("Search: " + std::to_string(m_entitiesinSearchArea.size())).c_str(), 10, 70, 20, BLACK);

    EndDrawing();
}

void Simulation::onResize(int width, int height) {
    Log::info(("New Size: " + std::to_string(width) + " : " + std::to_string(height)));
}
