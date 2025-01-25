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

#include "Tests/SimdSortTest.h"
#include "Tests/Test_ThreadPool.h"

#if defined(PLATFORM_WEB)

const WorldSettings worldSettings;
init(worldSettings, 1000000, 100, 100, 1024);

const ThreadSettings threadSettings;
init(threadSettings, worldSettings);

const ThreadPool threadPool(threadSettings);

Simulation& getInstance() {
    static Simulation simulation(worldSettings, threadSettings, threadPool);
    return simulation;
}

void UpdateCanvasSize(int width, int height) {
    getInstance().onResize(width, height);
}

void webTearDown() {
    emscripten_cancel_main_loop();

    threadPool.~ThreadPool();
    destroy(threadSettings);

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
        init(worldSettings, 1000000, 100, 100, 1024);

        ThreadSettings threadSettings;
        init(threadSettings, worldSettings);

        ThreadPool threadPool(threadSettings);

        createViewPort(false);
        Simulation simulation(worldSettings, threadSettings, threadPool);
        simulation.run();
        destroy(threadSettings);
    }
    CloseWindow();
#endif

    return 0;
}
