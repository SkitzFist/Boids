#ifndef BOIDS_ACCELERATION_H
#define BOIDS_ACCELERATION_H

#include "AlignedAllocator.h"

struct Accelerations {
    AlignedFloatVector x;
    AlignedFloatVector y;
};

inline void init(Accelerations& velocities, const int entityCount) {
    velocities.x.resize(entityCount);
    velocities.y.resize(entityCount);
}

#endif
