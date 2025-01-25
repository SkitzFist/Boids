#ifndef BOIDS_MORTON_CODE_H
#define BOIDS_MORTON_CODE_H

#include <stdint.h>

inline constexpr uint32_t partBy1(uint32_t x) {
    // x = ---- ---- ---- ---- ---- ---- ---- dcba (binary)
    x &= 0x0000ffff;
    x = (x ^ (x << 8)) & 0x00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f;
    x = (x ^ (x << 2)) & 0x33333333;
    x = (x ^ (x << 1)) & 0x55555555;
    return x;
}

inline constexpr uint32_t morton2D(uint32_t x, uint32_t y) {
    return (partBy1(y) << 1) | partBy1(x);
}

struct MortonEntry {
    uint64_t entityId;
    uint32_t mortonCode;

    MortonEntry() : entityId(0), mortonCode(0) {}
};

struct MortonRect {
    uint32_t xMin, yMin, xMax, yMax;
};

#endif
