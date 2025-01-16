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

    // 1,3
    __m256i idxLocalLong = _mm256_set_epi32(4, 5, 6, 7, 0, 1, 2, 3);
    constexpr int blendLocal = createMask(0, 0, 1, 1, 0, 0, 1, 1);

    // 2,4
    __m256i idxInter = _mm256_set_epi32(6, 7, 2, 3, 4, 5, 0, 1);
    constexpr int blendInter = createMask(0, 1, 0, 0, 1, 1, 0, 1);

    // 5
    __m256i idxLocalShort = _mm256_set_epi32(7, 5, 6, 3, 4, 1, 2, 0);
    constexpr const int blendLocalShort = createMask(0, 0, 1, 0, 1, 0, 1, 1);

    // 6
    __m256i idxPair = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
    constexpr const int blendPair = createMask(0, 1, 0, 1, 0, 1, 0, 1);

    // 1
    __m256i b = _mm256_permutevar8x32_epi32(reg, idxLocalLong);
    __m256i min = _mm256_min_epi32(reg, b);
    __m256i max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendLocal);
    // std::cout << "---1---\n";
    // print_m256i(reg);

    // 2
    b = _mm256_permutevar8x32_epi32(reg, idxInter);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendInter);
    // std::cout << "---2---\n";
    // print_m256i(reg);

    // 3
    b = _mm256_permutevar8x32_epi32(reg, idxLocalLong);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendLocal);
    // std::cout << "---3---\n";
    // print_m256i(reg);

    // 4
    b = _mm256_permutevar8x32_epi32(reg, idxInter);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendInter);

    // 5
    b = _mm256_permutevar8x32_epi32(reg, idxLocalShort);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendLocalShort);
    // std::cout << "---4---\n";
    // print_m256i(reg);

    // 6
    b = _mm256_permutevar8x32_epi32(reg, idxPair);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendPair);
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
