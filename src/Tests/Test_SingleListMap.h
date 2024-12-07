#ifndef TEST_SINGLE_LIST_MAP_H
#define TEST_SINGLE_LIST_MAP_H

#include "AlignedAllocator.h"
#include "Positions.h"
#include "Settings.h"
#include "SingleListMap.h"
#include "ThreadPool.h"
#include "Timer.h"

#include <iostream>

// void SingelListMapTest() {

//     std::cout << "Test Start!\n";
//     ThreadPool pool;
//     SingleListMap map;

//     Positions pos;
//     for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {
//         pos.x[i] = (float)GetRandomValue(0, WorldSettings::WORLD_WIDTH - 1);
//         pos.y[i] = (float)GetRandomValue(0, WorldSettings::WORLD_HEIGHT - 1);
//     }
//     using namespace std::literals::chrono_literals;
//     int nrOfTestFrames = 1000;
//     for (int i = 0; i < nrOfTestFrames; ++i) {
//         map.rebuild(pos);
//         pool.await();
//     }

//     pool.close();

//     std::cout << "TEST DONE!\n";
// }

void SingelListMapTest() {

    std::cout << "Test Start!\n";
    ThreadPool pool;
    SingleListMap map;

    Positions pos;
    for (int i = 0; i < WorldSettings::ENTITY_COUNT; ++i) {
        pos.x[i] = (float)GetRandomValue(0, WorldSettings::WORLD_WIDTH - 1);
        pos.y[i] = (float)GetRandomValue(0, WorldSettings::WORLD_HEIGHT - 1);
    }

    int nrOfTestFrames = 1000;
    for (int i = 0; i < nrOfTestFrames; ++i) {
        map.rebuild(pool, pos);
    }

    pool.await();

    // pool.close();

    std::cout << "TEST DONE!\n";
}

#endif
