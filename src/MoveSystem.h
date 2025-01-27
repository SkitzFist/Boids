#ifndef BOIDS_MOVE_SYSTEM_H
#define BOIDS_MOVE_SYSTEM_H

#include "AlignedAllocator.h"
#include "Positions.h"
#include "ThreadPool.h"
#include "Velocities.h"

void applyVelocities(ThreadPool& threadPool,
                     Positions& positions,
                     Velocities& velocities,
                     const int workerThreadCount,
                     const float worldWidth,
                     const float worldHeight,
                     const float dt);

void applyVelocitiesJob(AlignedFloatVector& pos,
                        AlignedFloatVector& vel,
                        const int entitiesStart,
                        const int entitiesEnd,
                        const float dt,
                        const float max);

#endif
