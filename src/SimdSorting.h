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

void sortRegister(__m256i& reg) {
    __m256i a = _mm256_load_si256(&reg);
    __m256i b = _mm256_load_si256(&reg);

    __m256i idx_comp = _mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1);
    __m256i idx_short = _mm256_set_epi32(7, 5, 6, 4, 3, 1, 2, 0);
    __m256i idx_long = _mm256_set_epi32(7, 5, 6, 3, 4, 1, 2, 0);
    __m256i min, max;

    // 1. compare min/max with a/b
    std::cout << "1. compare min/max with a/b\n";
    b = _mm256_permutevar8x32_epi32(b, idx_comp);
    min = _mm256_min_epi32(a, b);
    max = _mm256_max_epi32(a, b);
    print_m256i(a);
    // print_m256i(b);
    a = _mm256_blend_epi32(min, max, 0xAA);

    print_m256i(a);

    // 2. short swap
    std::cout << "\n2. Short swap\n";
    a = _mm256_permutevar8x32_epi32(a, idx_short);
    b = _mm256_permutevar8x32_epi32(a, idx_comp);
    print_m256i(a);
    // print_m256i(b);
    min = _mm256_min_epi32(a, b);
    max = _mm256_max_epi32(a, b);
    a = _mm256_blend_epi32(min, max, 0xAA);

    print_m256i(a);

    // 3. long swap
    std::cout << "\n3. Long swap\n";
    a = _mm256_permutevar8x32_epi32(a, idx_long);
    b = _mm256_permutevar8x32_epi32(a, idx_comp);
    print_m256i(a);
    // print_m256i(b);
    min = _mm256_min_epi32(a, b);
    max = _mm256_max_epi32(a, b);
    a = _mm256_blend_epi32(min, max, 0xAA);
    print_m256i(a);

    // 3. short swap
    std::cout << "\n4. Short swap\n";
    a = _mm256_permutevar8x32_epi32(a, idx_short);
    b = _mm256_permutevar8x32_epi32(a, idx_comp);
    print_m256i(a);
    // print_m256i(b);
    min = _mm256_min_epi32(a, b);
    max = _mm256_max_epi32(a, b);
    a = _mm256_blend_epi32(min, max, 0xAA);
    print_m256i(a);

    // from here it's three branches
    // A. Fully sorted
    // B. 0-4 & 5-7 is sorted, but not 0-7, solution: long swap then short swap.
    // C. 0-4 is not sorted 5-7 is sorted

    // if C is true, even if I havn't seen it yet in testing, I guess 0-4 being
    // sorted and 5-7 not sorted could also be possible so possible 4 branches

    reg = a;
}

void simdSort(std::vector<int, AlignedAllocator<int, 32>>& vec) {
    // __m256i a = _mm256_load_si256((__m256i*)&vec[0]);
    // __m256i b = _mm256_load_si256((__m256i*)&vec[8]);

    // sortRegister(a);
    // sortRegister(b);

    // _mm256_store_si256((__m256i*)&vec[0], a);
    // _mm256_store_si256((__m256i*)&vec[8], b);

    // __m256i a = _mm256_set_epi32(12, 10, 32, 17, 19, 9, 6, 7);
    __m256i a = _mm256_set_epi32(6, 3, 27, 18, 11, 26, 13, 9);
    sortRegister(a);

    std::cout << "\nResult\n";
    print_m256i(a);
}

#endif
