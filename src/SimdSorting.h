#ifndef BOIDS_SIMD_SORTING_H
#define BOIDS_SIMD_SORTING_H

#include <immintrin.h>
#include <iostream>
#include <vector>

/*
    I have a gut feeling it's possible to simd sort a register in 5-6 steps.
    Which is less then what known mathematics say. I've spent two weeks on this now,
    but havn't solved it yet, it feels like I'm getting closer though.
    But I'll have to leave it for now and return on a later date.
    TODO: Move this out to it's own repo.
*/

void print_m256i(__m256i reg) {
    alignas(32) int vals[8];
    _mm256_store_si256((__m256i*)vals, reg);
    for (int i = 0; i < 8; ++i) {
        std::cout << vals[i] << " ";
    }
    std::cout << std::endl;
}

constexpr int createMask(int a, int b, int c, int d, int e, int f, int g, int h) {
    return (a ? 1 : 0) << 0 |
           (b ? 1 : 0) << 1 |
           (c ? 1 : 0) << 2 |
           (d ? 1 : 0) << 3 |
           (e ? 1 : 0) << 4 |
           (f ? 1 : 0) << 5 |
           (g ? 1 : 0) << 6 |
           (h ? 1 : 0) << 7;
}

inline void doStep(__m256i& a, __m256i& b, __m256i& idx, int& blend) {
    b = _mm256_permutevar8x32_epi32(a, idx);
    __m256i min = _mm256_min_epi32(a, b);
    __m256i max = _mm256_max_epi32(a, b);
    a = _mm256_blend_epi32(min, max, blend);
}

/*
    __m256i idxPairOdd = _mm256_set_epi32(7, 5, 6, 3, 4, 1, 2, 0);
    constexpr const int blendPairOdd = createMask(0, 0, 1, 0, 1, 0, 1, 1);
*/

__m256i sortRegister(__m256i reg) {
    // print_m256i(reg);

    /*
    A: [(0,2),(1,3),(4,6),(5,7)]

    B: [(0,4),(1,5),(2,6),(3,7)]

    C: [(0,1),(2,3),(4,5),(6,7)]

    D: [(2,4),(3,5)]

    E: [(1,4),(3,6)]

    F: [(1,2),(3,4),(5,6)]
    */

    __m256i idxA = _mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2);
    constexpr int blendA = createMask(0, 0, 1, 1, 0, 0, 1, 1);

    __m256i idxB = _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4);
    constexpr int blendB = createMask(0, 0, 0, 0, 1, 1, 1, 1);

    __m256i idxC = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
    constexpr const int blendC = createMask(0, 1, 0, 1, 0, 1, 0, 1);

    __m256i idxD = _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0);
    constexpr const int blendD = createMask(0, 0, 0, 0, 1, 1, 1, 1);

    __m256i idxE = _mm256_set_epi32(7, 3, 5, 1, 6, 2, 4, 0);
    constexpr const int blendE = createMask(0, 0, 0, 0, 1, 1, 1, 1);

    __m256i idxF = _mm256_set_epi32(7, 5, 6, 3, 4, 1, 2, 0);
    constexpr const int blendF = createMask(0, 0, 1, 0, 1, 0, 1, 1);

    // 1
    __m256i b = _mm256_permutevar8x32_epi32(reg, idxA);
    __m256i min = _mm256_min_epi32(reg, b);
    __m256i max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendA);
    // std::cout << "---1---\n";
    // print_m256i(reg);

    // 2
    b = _mm256_permutevar8x32_epi32(reg, idxB);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendB);
    // std::cout << "---2---\n";
    // print_m256i(reg);

    // 3
    b = _mm256_permutevar8x32_epi32(reg, idxC);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendC);
    // std::cout << "---3---\n";
    // print_m256i(reg);

    // 4
    b = _mm256_permutevar8x32_epi32(reg, idxD);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendD);

    // 5
    b = _mm256_permutevar8x32_epi32(reg, idxE);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendE);
    // std::cout << "---4---\n";
    // print_m256i(reg);

    // 6
    b = _mm256_permutevar8x32_epi32(reg, idxF);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendF);
    // std::cout << "---5---\n";
    // print_m256i(reg);

    return reg;
}

void simdSort(std::vector<int, AlignedAllocator<int, 32>>& vec) {
    for (std::size_t i = 0; i < vec.size() - 7; i += 8) {
        // std::cout << "\n\n";
        _mm256_store_si256((__m256i*)&vec[i], sortRegister(_mm256_load_si256((__m256i*)&vec[i])));
    }

    // sortRegister(_mm256_set_epi32(6, 3, 27, 18, 11, 26, 13, 9));
}

#endif
