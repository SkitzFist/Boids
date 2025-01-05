#ifndef TESTS_SIMD_SORTING_H
#define TESTS_SIMD_SORTING_H
#include "SimdSorting.h"

#include <cstdlib>
#include <immintrin.h>
#include <iostream>
#include <vector>

#include "AlignedAllocator.h"
#include "Timer.h"

void print(std::vector<int, AlignedAllocator<int, 32>>& vec, std::vector<int>& original) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i % 8 == 0 && i != 0) {
            std::cout << "--------\n";
        }
        std::cout << "[" << i << "]: " << vec[i] << " : " << original[i] << "\n";
    }

    std::cout << std::endl;
}

void simdSortingTest() {
    Timer timer;

    int count = 16;
    std::vector<int, AlignedAllocator<int, 32>> vec;
    std::vector<int> original;
    vec.reserve(count);

    for (int i = 0; i < count; ++i) {
        vec.emplace_back(std::rand() % 32);
        // vec.emplace_back(count - i);
        original.emplace_back(vec[i]);
    }

    timer.begin();
    simdSort(vec);
    std::cout << "Sorted completed in: " << timer.getDuration() << "\n\n";
    // print(vec, original);
}

#endif
