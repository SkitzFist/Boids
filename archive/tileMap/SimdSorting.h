#ifndef BOIDS_SIMD_SORTING_H
#define BOIDS_SIMD_SORTING_H

#include <immintrin.h>
#include <iostream>
#include <vector>

/*
    Ok I managed to solve this in 6 steps, not sure if it's possible to go lower.
    Anyway, this type of sorting won't be of use in this project, unless I can crack
    how to certainly store what index swaps is happening so it can be reflected in the
    entityId array.
    This only sorts on register basis, need to figure out how to sort two registers with eachother.
    TODO: Move this out to it's own repo.
*/
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

#if defined(EMSCRIPTEN)
void simdSort(std::vector<int, AlignedAllocator<int, 32>>& vec) {}
#else

void print_m256i(__m256i reg) {
    alignas(32) int vals[8];
    _mm256_store_si256((__m256i*)vals, reg);
    for (int i = 0; i < 8; ++i) {
        std::cout << vals[i] << " ";
    }
    std::cout << std::endl;
}

__m256i sortRegister(__m256i reg) {
    // print_m256i(reg);

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

    // 2
    b = _mm256_permutevar8x32_epi32(reg, idxB);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendB);

    // 3
    b = _mm256_permutevar8x32_epi32(reg, idxC);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendC);

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

    // 6
    b = _mm256_permutevar8x32_epi32(reg, idxF);
    min = _mm256_min_epi32(reg, b);
    max = _mm256_max_epi32(reg, b);
    reg = _mm256_blend_epi32(min, max, blendF);

    return reg;
}

void simdSort(std::vector<int, AlignedAllocator<int, 32>>& vec) {
    for (std::size_t i = 0; i < vec.size() - 7; i += 8) {
        _mm256_store_si256((__m256i*)&vec[i], sortRegister(_mm256_load_si256((__m256i*)&vec[i])));
    }
}

#endif

#endif
