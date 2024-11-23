#ifndef BOIDS_POSITIONS_H
#define BOIDS_POSITIONS_H

#include <vector>

#include "SimulationSettings.h"

struct Positions {
    std::vector<float> x;
    std::vector<float> y;

    Positions() {
        x.resize(WorldSettings::ENTITY_COUNT);
        y.resize(WorldSettings::ENTITY_COUNT);
    }
};

#endif
