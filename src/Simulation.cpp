#include "Simulation.h"

#include "CameraMoveSystem.h"
#include "Log.h"
#include "Settings.h"
#include "ThreadPool.h"
#include "raylib.h"
#include <immintrin.h>

inline void test(int id) {
    Log::info("id: " + std::to_string(id));
}

Simulation::Simulation() : m_threadPool(),
                           m_camera(),
                           /*m_tileMap(m_threadPool.numThreads)
                           m_tileMapSimd(WorldSettings::TILE_COUNT, WorldSettings::ENTITY_COUNT)*/
                           m_singleListMap() {
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
        m_positions.x[i] = GetRandomValue(0, WorldSettings::WORLD_WIDTH - 32);
        m_positions.y[i] = GetRandomValue(0, WorldSettings::WORLD_HEIGHT - 32);
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

    // rebuild tile map

    const int batchSize = 1000000;
    const int entitiesPerThread = batchSize / ThreadSettings::THREAD_COUNT;
    for (int batch = 0; batch < 10; ++batch) {
        int startEntityBatch = batch * batch;

        for (int thread = 0; thread < ThreadSettings::THREAD_COUNT; ++thread) {
            int startEntity = startEntityBatch + (entitiesPerThread * thread);
            int endEntity = startEntity + entitiesPerThread;

            m_threadPool.enqueue(thread, [startEntity, endEntity, &entityToTile = this->m_singleListMap.entityToTile, &pos = this->m_positions] {
                __m256 tileWidthVec = _mm256_set1_ps(WorldSettings::TILE_WIDTH);
                __m256 tileHeightVec = _mm256_set1_ps(WorldSettings::TILE_HEIGHT);
                __m256i worldColumnsVec = _mm256_set1_epi32(WorldSettings::WORLD_COLUMNS);
                __m256 invTileWidthVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_WIDTH);
                __m256 invTileHeightVec = _mm256_set1_ps(1.0f / WorldSettings::TILE_HEIGHT);
                int entity = startEntity;

                for (; entity < endEntity - 8; entity += 8) {
                    // preftch
                    _mm_prefetch((const char*)&pos.x[entity + 16], _MM_HINT_T0);
                    _mm_prefetch((const char*)&pos.y[entity + 16], _MM_HINT_T0);

                    __m256 xPosVec = _mm256_loadu_ps(&pos.x[entity]);
                    __m256 yPosVec = _mm256_loadu_ps(&pos.y[entity]);

                    __m256 xMul = _mm256_mul_ps(xPosVec, invTileWidthVec);
                    __m256 yMul = _mm256_mul_ps(yPosVec, invTileHeightVec);

                    __m256i tileColVec = _mm256_cvttps_epi32(xMul);
                    __m256i tileRowVec = _mm256_cvttps_epi32(yMul);

                    __m256i tileRowMul = _mm256_mullo_epi32(tileRowVec, worldColumnsVec);
                    __m256i tileIndexVec = _mm256_add_epi32(tileRowMul, tileColVec);

                    _mm256_storeu_si256((__m256i*)&entityToTile[entity], tileIndexVec);
                }
            });
        }
        // m_threads.awaitCompletion();
        m_threadPool.await();
    }
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
