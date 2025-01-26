#ifndef BOIDS_CORE_AFFINITY_H
#define BOIDS_CORE_AFFINITY_H

#ifdef __EMSCRIPTEN__
// Emscripten/WebAssembly does not have direct access to control CPU affinity as it's designed for the web
#include "Log.h"
#include <emscripten/emscripten.h>

inline void setAffinity(const int core) {
    // No operation: WebAssembly runs within a browser environment, and thread affinity is managed by the JS engine/browser.
    Log::info("Setting CPU affinity is not supported with WebAssembly\n");
}

inline void setPriority() {
    Log::info("Setting CPU priority is not supported with WebAssembly\n");
}

#else

#ifdef _WIN32
#include <windows.h>

inline void setAffinity(const int core) {
    HANDLE current_thread = GetCurrentThread();
    DWORD_PTR affinity_mask = core; // Pin to core 0
    if (SetThreadAffinityMask(current_thread, affinity_mask) == 0) {
        printf("Failed to set thread affinity on Windows\n");
    }
}

inline void setPriority() {
    HANDLE current_thread = GetCurrentThread();
    if (!SetThreadPriority(current_thread, THREAD_PRIORITY_HIGHEST)) {
        printf("Failed to set thread priority on Windows\n");
    }
}

#elif defined(__linux__)
#include <pthread.h>
#include <sched.h>
#include <stdio.h>

inline void setAffinity(const int core) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        printf("Failed to set thread affinity on Linux\n");
    }
}

inline void setPriority() {
    pthread_t this_thread = pthread_self();
    struct sched_param params;
    params.sched_priority = 80; // Higher priority

    if (pthread_setschedparam(this_thread, SCHED_RR, &params) != 0) {
        printf("Failed to set thread priority on Linux\n");
    }
}

#endif
#endif
#endif