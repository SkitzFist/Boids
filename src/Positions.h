#ifndef BOIDS_POSITIONS_H
#define BOIDS_POSITIONS_H

#include <vector>

#include "AlignedAllocator.h"
#include "Settings.h"

struct Positions {
    AlignedFloatVector x;
    AlignedFloatVector y;
};

inline void init(Positions& pos, const int entityCount) {
    pos.x.resize(entityCount);
    pos.y.resize(entityCount);
}

#endif
