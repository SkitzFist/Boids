#ifndef BOIDS_LOG_H
#define BOIDS_LOG_H

#include <string>

#include "raylib.h"

namespace Log {

inline void info(const std::string& log) {
    TraceLog(LOG_INFO, log.c_str());
}

inline void vector2(std::string name, const Vector2& vec) {
    std::string log = name + ": " + std::to_string(vec.x) + " : " + std::to_string(vec.y);
    info(log);
}

} // namespace Log

#endif
