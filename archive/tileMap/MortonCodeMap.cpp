#include "MortonCodeMap.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <future>

#include "Log.h"

void init(MortonMap& map, const int entityCount) {
    map.entries.resize(entityCount);
    map.entriesCpy.resize(entityCount);
    map.entriesPtr.reserve(entityCount);
    map.ids.resize(entityCount);

    for (int i = 0; i < entityCount; ++i) {
        map.ids[i] = i;
        map.entriesPtr.push_back(&map.entries[i]);
    }
}

void encode(MortonMap& map, ThreadPool& pool, const PositionsI& pos) {

    int startEntity, endEntity;
    const int threadCount = ThreadSettingsNameSpace::THREAD_COUNT;
    const int entitiesPerThread = ThreadSettingsNameSpace::ENTITIES_PER_THREAD;
    for (int thread = 0; thread < threadCount; ++thread) {
        startEntity = entitiesPerThread * thread;
        endEntity = startEntity + entitiesPerThread;

        pool.enqueue(thread, [&entries = map.entries, &pos, startEntity, endEntity] {
            for (int entity = startEntity; entity < endEntity; ++entity) {
                entries[entity] = morton2D(pos.x[entity], pos.y[entity]);
            }
        });
    }

    pool.await();

    // memcpy(map.entriesCpy.data(), map.entries.data(), map.entries.size() * sizeof(uint32_t));
    // sort(map.entriesCpy, map.ids);
}

//////////////////////////
///       SORTING       //
/////////////////////////

bool needsSorting(std::vector<uint32_t*>& entries, ThreadPool& pool) {
    std::atomic<bool> needsSorting = false;

    int startEntity, endEntity;
    for (int thread = 0; thread < ThreadSettingsNameSpace::THREAD_COUNT; ++thread) {
        startEntity = thread * ThreadSettingsNameSpace::ENTITIES_PER_THREAD;
        // additional -1 because it compares i > i + 1
        endEntity = startEntity + ThreadSettingsNameSpace::ENTITIES_PER_THREAD - 1;

        pool.enqueue(thread, [&entries, &needsSorting, startEntity, endEntity] {
            int i = startEntity;

            while (i < endEntity) {
                if (needsSorting.load(std::memory_order_relaxed)) {
                    return;
                }

                if (*entries[i] > *entries[i + 1]) {
                    needsSorting = true;
                    return;
                }
                ++i;
            }
        });
    }

    pool.await();

    return needsSorting;
}

void sort(std::vector<uint32_t*>& entries, std::vector<int>& ids) {
    // std::sort(std::execution::par, entries.begin(), entries.end(),
    //           [](const uint32_t* a, const uint32_t* b) {
    //               return *a > *b;
    //           });
}

void sort(std::vector<uint32_t>& entries, std::vector<int>& ids) {
    // std::sort(std::execution::par, entries.begin(), entries.end());
}

//////////////////////////
///       QUERY         //
/////////////////////////

void search(MortonMap& map, const MortonRect& rect, std::vector<int>& result, const PositionsI& pos) {
    uint32_t codeMin = morton2D(rect.xMin, rect.yMin);
    uint32_t codeMax = morton2D(rect.xMax, rect.yMax);

    if (codeMin > codeMax) {
        std::swap(codeMin, codeMax);
    }

    auto lowerIt = std::lower_bound(map.entriesPtr.begin(), map.entriesPtr.end(),
                                    codeMin,
                                    [](const uint32_t* entry, uint32_t c) {
                                        return *entry < c;
                                    });

    auto upperIt = std::upper_bound(lowerIt, map.entriesPtr.end(),
                                    codeMax,
                                    [](uint32_t c, const uint32_t* entry) {
                                        return c < *entry;
                                    });

    uint32_t* entry;
    int id;
    for (auto it = lowerIt; it != upperIt; ++it) {
        entry = *it;
        // id = map.ids[it]; TODO needs to fix this as ids now is separated to its own vector.

        if (pos.x[id] > rect.xMin &&
            pos.x[id] < rect.xMax &&
            pos.y[id] > rect.yMin &&
            pos.y[id] < rect.yMax) {
            result.emplace_back(id);
        }
    }
}