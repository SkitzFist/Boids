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

struct PositionsI {
    AlignedInt32Vector x;
    AlignedInt32Vector y;

    PositionsI() {
        x.resize(WorldSettings::ENTITY_COUNT);
        y.resize(WorldSettings::ENTITY_COUNT);
    }
};

struct PositionsI16 {
    std::vector<uint16_t> x;
    std::vector<uint16_t> y;

    PositionsI16() {
        x.resize(WorldSettings::ENTITY_COUNT);
        y.resize(WorldSettings::ENTITY_COUNT);
    }
};

#endif
