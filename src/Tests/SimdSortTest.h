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

    int count = 24; // 1000256;
    std::vector<int, AlignedAllocator<int, 32>>
        vec;
    std::vector<int> original;
    vec.reserve(count);
    original.reserve(count);

    for (int i = 0; i < count; ++i) {
        vec.emplace_back(std::rand() % 32);
        // vec.emplace_back(count - i);
        original.emplace_back(vec[i]);
    }
    double duration;
    timer.begin();
    // std::sort(vec.begin(), vec.end());
    simdSort(vec);
    duration = timer.getDuration();
    // print(vec, original);
    std::cout << "Sorted completed in: " << duration << "\n\n";
    int intraRegisterFails = 0;
    for (int i = 0; i < vec.size(); i += 8) {
        for (int j = i; j < i + 7; ++j) { // Check within the register
            if (vec[j] > vec[j + 1]) {
                ++intraRegisterFails;
                for (int k = i; k < i + 8; ++k) {
                    std::cout << "[" << k << "]: " << vec[k] << "  :  " << original[k] << "\n";
                }
                return;
            }
        }
    }
    std::cout << "Total intra-register failures: " << intraRegisterFails << "\n";
}

#endif
