#ifndef BOIDS_SIMD_SORTING_H
#define BOIDS_SIMD_SORTING_H

#include <immintrin.h>
#include <iostream>
#include <vector>

void print_m256i(__m256i reg) {
    alignas(32) int vals[8];
    _mm256_store_si256((__m256i*)vals, reg);
    for (int i = 0; i < 8; ++i) {
        std::cout << vals[i] << " ";
    }
    std::cout << std::endl;
}

inline void swapIfGreater(__m256i& a, __m256i& idx, __m256i idx_shift) {
    __m256i idx_original = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
    __m256i b = _mm256_permutevar8x32_epi32(a, idx);
    __m256i comp = _mm256_cmpgt_epi32(a, b);
    comp = _mm256_permutevar8x32_epi32(comp, idx_shift);

    __m256i idx_final = _mm256_blendv_epi8(idx_original, idx, comp);

    a = _mm256_permutevar8x32_epi32(a, idx_final);
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

__m256i sortRegister(__m256i reg) {
    // print_m256i(reg);

    __m256i idxLocal = _mm256_set_epi32(4, 5, 6, 7, 0, 1, 2, 3);
    int blendLocal = createMask(0, 0, 1, 1, 0, 0, 1, 1);

    __m256i idxInterOdd = _mm256_set_epi32(6, 7, 2, 3, 4, 5, 0, 1);
    int blendInterOdd = createMask(0, 1, 0, 0, 1, 1, 0, 1);

    __m256i idxInterEven = _mm256_set_epi32(6, 7, 3, 2, 5, 4, 0, 1);
    int blendInterEven = createMask(0, 1, 0, 0, 1, 1, 0, 1);

    __m256i idxNeighbour = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
    int blendNeighbour = createMask(0, 1, 0, 1, 0, 1, 0, 1);

    // 1
    __m256i b = _mm256_permutevar8x32_epi32(reg, idxLocal);
    __m256i min = _mm256_min_epi32(reg, b);
    __m256i max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendLocal);
    std::cout << "---1---\n";
    print_m256i(reg);

    // 2
    b = _mm256_permutevar8x32_epi32(reg, idxInterOdd);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendInterOdd);
    std::cout << "---2---\n";
    print_m256i(reg);

    // 3
    b = _mm256_permutevar8x32_epi32(reg, idxLocal);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendLocal);
    std::cout << "---3---\n";
    print_m256i(reg);

    // 4
    b = _mm256_permutevar8x32_epi32(reg, idxInterEven);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendInterEven);
    std::cout << "---4---\n";
    print_m256i(reg);

    // 5
    b = _mm256_permutevar8x32_epi32(reg, idxNeighbour);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendNeighbour);
    std::cout << "---5---\n";
    print_m256i(reg);

    return reg;
}

void simdSort(std::vector<int, AlignedAllocator<int, 32>>& vec) {
    for (std::size_t i = 0; i < vec.size() - 7; i += 8) {
        std::cout << "\n\n";
        _mm256_store_si256((__m256i*)&vec[i], sortRegister(_mm256_load_si256((__m256i*)&vec[i])));
    }

    // sortRegister(_mm256_set_epi32(6, 3, 27, 18, 11, 26, 13, 9));
}

#endif
