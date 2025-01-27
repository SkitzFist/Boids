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
    int workerCount;
    int entitiesPerThread;
};

inline void init(ThreadSettings& threadSettings, WorldSettings& worldSettings) {
    int coresLeft = getPhysicalCoreCount() - 2;

    // what's left is worker threads.
    if (coresLeft <= 0) {
        // houston we got a problem
        std::cerr << "ThreadSettings: System has too few cores\n";
        exit(1);
    }

    threadSettings.workerCount = coresLeft;

    // Entity count needs to be evenly divisible with worker threads
    if (worldSettings.entityCount % (threadSettings.workerCount / 2) != 0) {
        worldSettings.entityCount += (threadSettings.workerCount / 2) - (worldSettings.entityCount % (threadSettings.workerCount / 2));
    }

    threadSettings.entitiesPerThread = worldSettings.entityCount / threadSettings.workerCount;
}

inline void destroy(ThreadSettings& settings) {
}

#endif
