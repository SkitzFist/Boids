#if defined(PLATFORM_WEB)
#include "emscripten.h"
#include <emscripten/bind.h>
#endif

#include "raylib.h"

#include "Simulation.h"
#include "ThreadPool.h"
#include "ThreadSettings.h"
#include "ViewPort.h"
#include "WorldSettings.h"

#include "Tests/Test_ThreadPool.h"

constexpr const int ENTITY_COUNT = 1000000;
constexpr const int COLUMNS = 50;
constexpr const int ROWS = 50;
constexpr const int TILE_SIZE = 1024;

#if defined(PLATFORM_WEB)

WorldSettings worldSettings;
ThreadSettings threadSettings;

ThreadPool* threadPool;

Simulation& getInstance() {
    static Simulation simulation(worldSettings, threadSettings, *threadPool);
    return simulation;
}

void UpdateCanvasSize(int width, int height) {
    getInstance().onResize(width, height);
}

void webTearDown() {
    emscripten_cancel_main_loop();

    delete threadPool;

    CloseWindow();
}

void webLoop() {
    if (getInstance().webRun()) {
        webTearDown();
    }
}

// bindings
EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("UpdateCanvasSize", &UpdateCanvasSize);
}

#endif

int main() {

    // ThreadPoolTest();
    // simdSortingTest();
    // return 0;

#if defined(PLATFORM_WEB)

    init(worldSettings, ENTITY_COUNT, COLUMNS, ROWS, TILE_SIZE);
    init(threadSettings, worldSettings);
    threadPool = new ThreadPool(threadSettings);

    createViewPort(true);

    emscripten_run_script(R"(
        window.onresize = function() {
            var canvas = document.getElementById('canvas');
            var width = window.innerWidth;
            var height = window.innerHeight;
            canvas.width = width;
            canvas.height = height;
            Module.UpdateCanvasSize(width, height);
        };
    )");

    emscripten_set_main_loop(webLoop, 0, 1);
#else
    {
        WorldSettings worldSettings;
        init(worldSettings, ENTITY_COUNT, COLUMNS, ROWS, TILE_SIZE);

        ThreadSettings threadSettings;
        init(threadSettings, worldSettings);

        ThreadPool threadPool(threadSettings);

        setAffinity(threadSettings.workerCount);

        createViewPort(false);
        Simulation simulation(worldSettings, threadSettings, threadPool);
        simulation.run();
    }
    CloseWindow();
#endif

    return 0;
}
