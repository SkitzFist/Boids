#include "MoveSystem.h"

#include <immintrin.h>

#include "Log.h"

void applyVelocities(ThreadPool& threadPool,
                     Positions& positions,
                     Velocities& velocities,
                     const int threadCount,
                     const float worldWidth,
                     const float worldHeight,
                     float dt) {

    int threads = threadCount - 2;
    int entitiesPerThread = positions.x.size() / threads;
    for (int i = 0; i < threads; ++i) {
        int entitiesStart = i * entitiesPerThread;
        int entitiesEnd = entitiesStart + entitiesPerThread;

        int thread = 1 + i; // uses upper half of threadpool
        threadPool.enqueue(thread,
                           applyVelocitiesJob,
                           std::ref(positions.x),
                           std::ref(velocities.x),
                           entitiesStart,
                           entitiesEnd,
                           dt,
                           worldWidth);

        threadPool.enqueue(thread,
                           applyVelocitiesJob,
                           std::ref(positions.y),
                           std::ref(velocities.y),
                           entitiesStart,
                           entitiesEnd,
                           dt,
                           worldHeight);
    }
    threadPool.awaitWorkers(1, threadCount);
}

void applyVelocitiesJob(AlignedFloatVector& pos,
                        AlignedFloatVector& vel,
                        const int entitiesStart,
                        const int entitiesEnd,
                        const float dt,
                        const float max) {
    __m256 velocitiesVec;
    __m256 positionsVec;
    __m256 addedVec;
    __m256 dtVec = _mm256_set1_ps(dt);
    __m256 minVec = _mm256_set1_ps(0.0f);
    __m256 maxVec = _mm256_set1_ps(max);

    int entity = entitiesStart;
    for (; entity <= entitiesEnd - 8; entity += 8) {
        _mm_prefetch((const char*)&pos[entity + 16], _MM_HINT_T0);
        _mm_prefetch((const char*)&vel[entity + 16], _MM_HINT_T0);

        positionsVec = _mm256_load_ps(&pos[entity]);
        velocitiesVec = _mm256_load_ps(&vel[entity]);

        velocitiesVec = _mm256_mul_ps(velocitiesVec, dtVec);
        addedVec = _mm256_add_ps(positionsVec, velocitiesVec);

        addedVec = _mm256_max_ps(addedVec, minVec);
        addedVec = _mm256_min_ps(addedVec, maxVec);

        _mm256_store_ps(&pos[entity], addedVec);
    }

    for (; entity < entitiesEnd; ++entity) {
        pos[entity] += vel[entity] * dt;
        pos[entity] = std::max(0.0f, std::min(pos[entity], max));
    }
}