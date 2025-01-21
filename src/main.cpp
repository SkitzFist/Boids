#if defined(PLATFORM_WEB)
#include "emscripten.h"
#include <emscripten/bind.h>
#endif

#include "raylib.h"

#include "Simulation.h"
#include "ViewPort.h"

#include "CoreAffinity.h"

#include "Tests/SimdSortTest.h"
#include "Tests/Test_SingleListMap.h"
#include "Tests/Test_ThreadPool.h"
#include "Tests/Test_ThreadVector.h"

#if defined(PLATFORM_WEB)

Simulation& getInstance() {
    static Simulation simulation;
    return simulation;
}

void UpdateCanvasSize(int width, int height) {
    getInstance().onResize(width, height);
}

void webTearDown() {
    emscripten_cancel_main_loop();
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

    setAffinity(0);
    // setPriority();

    // test_threadVector();
    // SingelListMapTest();
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
        createViewPort(false);
        Simulation simulation;
        simulation.run();
    }
    CloseWindow();
#endif

    return 0;
}
