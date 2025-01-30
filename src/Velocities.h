#ifndef BOIDS_VELOCITIES_H
#define BOIDS_VELOCITIES_H

#include "AlignedAllocator.h"

struct Velocities {
    AlignedFloatVector x;
    AlignedFloatVector y;
};

inline void init(Velocities& velocities, const int entityCount) {
    velocities.x.resize(entityCount);
    velocities.y.resize(entityCount);
}

#endif
