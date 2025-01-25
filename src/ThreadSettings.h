#ifndef BOIDS_THREAD_SETTINGS_H
#define BOIDS_THREAD_SETTINGS_H

#include "CoreAffinity.h"
#include "WorldSettings.h"

#if defined(_WIN32)
#include <windows.h>

inline int getPhysicalCoreCount() noexcept {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<int>(sysInfo.dwNumberOfProcessors);
}

#elif defined(__linux__)

#include <unistd.h>

inline int getPhysicalCoreCount() noexcept {
    return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
}

#else
#include <thread>
// fallback to get logical cores (for webassembly etc..)
inline int getPhysicalCoreCount() noexcept {
    return static_cast<int>(std::thread::hardware_concurrency());
}

#endif

// debug
#include <iostream>

struct ThreadSettings {
    int tileMapThread;

    int workerStart;
    int workerCount;

    int entitiesPerThread;

    int* threadEntityIndexes;

    ThreadSettings() : tileMapThread(0), workerStart(0), workerCount(0), entitiesPerThread(0), threadEntityIndexes(nullptr) {}
    // Prevent copying
    ThreadSettings(const ThreadSettings&) = delete;
    ThreadSettings& operator=(const ThreadSettings&) = delete;
};

inline void init(ThreadSettings& threadSettings, WorldSettings& worldSettings) {
    int coresLeft = getPhysicalCoreCount();

    --coresLeft; // main thread/core
    --coresLeft; // tilemap thread/core

    // what's left is worker threads.
    if (coresLeft <= 0) {
        // houston we got a problem
        std::cerr << "ThreadSettings: System has too few cores\n";
        exit(1);
    } else if (coresLeft == 2) {
        // leave no cores for the system (will be a laggy ride though)
    } else if (coresLeft > 2) {
        // leave two cores for the system
        coresLeft -= 2;
    }

    threadSettings.tileMapThread = 0;
    threadSettings.workerStart = 1;
    threadSettings.workerCount = coresLeft;

    // Entity count needs to be evenly divisible with worker threads
    if (worldSettings.entityCount % threadSettings.workerCount != 0) {
        worldSettings.entityCount += threadSettings.workerCount - (worldSettings.entityCount % threadSettings.workerCount);
    }

    threadSettings.entitiesPerThread = worldSettings.entityCount / threadSettings.workerCount;

    // save start and end index
    int* threadEntityIndexes = new int[threadSettings.workerCount * 2];

    for (int i = 0; i < threadSettings.workerCount; ++i) {
        int start = i * threadSettings.entitiesPerThread;
        int end = start + threadSettings.entitiesPerThread;
        threadEntityIndexes[i * 2] = start;
        threadEntityIndexes[i * 2 + 1] = end;
    }
}

inline void destroy(ThreadSettings& settings) {
    delete[] settings.threadEntityIndexes;
}

#endif
