#include "Simulation.h"

#include <immintrin.h>

#include "CameraMoveSystem.h"
#include "DebugMortonView.h"
#include "DebugTIleIndex.h"
#include "Log.h"
#include "Settings.h"
#include "ThreadPool.h"
#include "raylib.h"

#include "Timer.h"

inline void test(int id) {
    Log::info("id: " + std::to_string(id));
}

Simulation::Simulation() : m_threadPool(),
                           m_camera(),
                           m_threadMap() {
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

    for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {
        m_positions.x[i] = GetRandomValue(0, WorldSettings::WORLD_WIDTH - 16);
        m_positions.y[i] = GetRandomValue(0, WorldSettings::WORLD_HEIGHT - 16);

        m_positionsI.x[i] = GetRandomValue(0, WorldSettings::WORLD_WIDTH - 16);
        m_positionsI.y[i] = GetRandomValue(0, WorldSettings::WORLD_HEIGHT - 16);
    }

    // init(m_tileMap, WorldSettings::ENTITY_COUNT, m_threadPool, m_positions);

    // init(m_mortonMap, WorldSettings::ENTITY_COUNT);
    // encode(m_mortonMap, m_threadPool, m_positionsI);
    // sort(m_mortonMap.entriesPtr, m_mortonMap.ids);

    init(m_threadMap, WorldSettings::ENTITY_COUNT);
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

    // tmp move pos
    // for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {

    //     int speed = i % 2 == 0 ? 1 : -1;
    //     m_positions.x[i] += GetRandomValue(0, speed);

    //     if (m_positions.x[i] > WorldSettings::WORLD_WIDTH) {
    //         m_positions.x[i] = 0;
    //     } else if (m_positions.x[i] < 0) {
    //         m_positions.x[i] = WorldSettings::WORLD_WIDTH - 32;
    //     }
    // }

    // for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {

    //     int speed = i % 2 == 0 ? 1 : -1;
    //     m_positionsI.x[i] += GetRandomValue(0, speed);

    //     if (m_positionsI.x[i] > WorldSettings::WORLD_WIDTH) {
    //         m_positionsI.x[i] = 0;
    //     } else if (m_positionsI.x[i] < 0) {
    //         m_positionsI.x[i] = WorldSettings::WORLD_WIDTH - 32;
    //     }
    // }

    m_entitiesInRange.clear();

    // rebuild tile map
    // m_singleListMap.rebuild(m_threadPool, m_positions); // 5.9 - 6.2 k fps || 188 fps
    // rebuildAndSort(m_tileMap, m_threadPool, m_positions);
    // rebuild(m_tileMap, m_threadPool, m_positions);
    // sort(m_tileMap, m_threadPool);
    // Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), m_camera);

    // int tileCol = mousePos.x / WorldSettings::TILE_WIDTH;
    // int tileRow = mousePos.y / WorldSettings::TILE_HEIGHT;
    // int index = tileRow * WorldSettings::WORLD_COLUMNS + tileCol;

    // m_timer.begin();
    // search(m_tileMap, m_threadPool, m_cameraRect, m_entitiesInRange);
    // m_timer.stop();

    // // threadMap
    m_buildTimer.begin();
    rebuild(m_threadMap, m_threadPool, m_positions);
    m_buildTimer.stop();

    // morton Map
    // encode(m_mortonMap, m_threadPool, m_positionsI);
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
    Rectangle src = {0.f, 0.f, 16.f, 16.f};
    Rectangle dst = {0.f, 0.f, 16.f, 16.f};

    // for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {
    //     dst.x = (float)m_positions.x[i];
    //     dst.y = (float)m_positions.y[i];
    //     DrawTexturePro(m_circleTexture, src, dst, {0.f, 0.f}, 0.f, RED);
    // }

    // for (const int i : m_entitiesInRange) {
    //     dst.x = (float)m_positions.x[i];
    //     dst.y = (float)m_positions.y[i];
    //     DrawTexturePro(m_circleTexture, src, dst, {0.f, 0.f}, 0.f, YELLOW);
    // }

    // drawMortonCodes(m_mortonMap);

    // DrawRectangleLines(m_searchArea.xMin, m_searchArea.yMin, m_searchArea.xMax, m_searchArea.yMax, GREEN);

    // draw camera rect
    // DrawRectangleLinesEx(m_cameraRect, 5.f, YELLOW);
    EndMode2D();

    // draw UI
    // draw(m_tileMap);

    DrawText(("FPS: " + std::to_string(GetFPS())).c_str(), 10, 10, 20, BLACK);
    DrawText(("Entities: " + std::to_string(WorldSettings::ENTITY_COUNT)).c_str(), 10, 30, 20, BLACK);
    DrawText(("Drawn: " + std::to_string(m_entitiesInRange.size())).c_str(), 10, 50, 20, BLACK);
    DrawText(("BuildTime: " + std::to_string(m_buildTimer.lastDuration)).c_str(), 10, 70, 20, BLACK);
    DrawText(("SearchTime: " + std::to_string(m_searchTimer.lastDuration)).c_str(), 10, 90, 20, BLACK);

    EndDrawing();
}

void Simulation::onResize(int width, int height) {
    Log::info(("New Size: " + std::to_string(width) + " : " + std::to_string(height)));
}
