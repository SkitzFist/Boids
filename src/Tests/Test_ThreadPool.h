#ifndef TEST_THREAD_POOL_H
#define TEST_THREAD_POOL_H

#include <iostream>

#include "ThreadPool.h"
#include "ThreadSettings.h"

constexpr const int threads = 6;
using namespace std::chrono_literals;
void ThreadPoolTest() {
    std::cout << "Starting ThreadPool test\n";
    ThreadSettings settings;
    settings.workerCount = 8;
    settings.workerStart = 1;

    ThreadPool pool(settings);

    pool.enqueue(0, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        std::cout << "TileMap thread Done\n";
    });

    for (int i = 1; i < settings.workerCount; ++i) {
        pool.enqueue(i, [i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 * i));
            std::cout << "WorkerThread[" << i << "]: done!\n";
        });
    }

    std::cout << "A\n";

    pool.awaitTileMap();

    std::cout << "B\n";

    pool.awaitWorkers();
    std::cout << "C\n";
}

#endif
