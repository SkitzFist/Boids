#ifndef BOIDS_POSITIONS_H
#define BOIDS_POSITIONS_H

#include <vector>

#include "AlignedAllocator.h"
#include "Settings.h"

struct Positions {
    AlignedFloatVector x;
    AlignedFloatVector y;

    Positions() {
        x.resize(WorldSettings::ENTITY_COUNT);
        y.resize(WorldSettings::ENTITY_COUNT);
    }
};

#endif
