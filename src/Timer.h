#ifndef BOIDS_TIMER_H
#define BOIDS_TIMER_H

#include <chrono>

struct Timer {
    std::chrono::high_resolution_clock::time_point start;
    double lastDuration = 0.0;

    void begin() {
        start = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        lastDuration = duration.count();
    }

    double getDuration() const {
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        return duration.count();
    }
};

#endif
