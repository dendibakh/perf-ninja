#include "solution.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <memory>
#include <numeric>

#ifdef __x86_64__
#include <immintrin.h>
#include <xmmintrin.h>
#else
#include <arm_neon.h>
#endif

#include <array>
#include <iostream>
#include <numeric>
#ifdef __x86_64__
void print_i16_vals(__m128i x) {
    int16_t arr[8];
    _mm_storeu_si128((__m128i_u *) arr, x);
    for (auto i: arr)
        std::cout << i << ' ';
    std::cout << '\n';
}
void test() {
    int16_t arr[16];
    std::iota(arr, arr + 16, 1);
    __m128i tmp = _mm_loadu_si128((__m128i_u *) arr);
    print_i16_vals(tmp);
    print_i16_vals(_mm_slli_si128(tmp, 2));
    std::cout << '\n';
}
#else
void print_i16_vals(uint16x8_t x) {
    int16_t arr[8];
    vst1q_u16((uint16x8_t *) arr, x);
    for (auto i: arr)
        std::cout << i << ' ';
    std::cout << '\n';
}
void test() {
    int16_t arr[16];
    std::iota(arr, arr + 16, 1);
    uint16x8_t tmp = vld1q_u16(arr);
    print_i16_vals(tmp);
    print_i16_vals(vextq_u16(tmp, vdupq_n_s16(0), 1));
    std::cout << '\n';
}
#endif

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
    int pos = 0;
    int currentSum = 0;
    int size = static_cast<int>(input.size());

    // 1. left border - time spend in this loop can be ignored, no need to
    // optimize it
    for (int i = 0; i < std::min<int>(size, radius); ++i) {
        currentSum += input[i];
    }

    int limit = std::min(radius + 1, size - radius);

    for (pos = 0; pos < limit; ++pos) {
        currentSum += input[pos + radius];
        output[pos] = currentSum;
    }

    // 2. main loop.
    limit = size - radius;
#define USEAVX2 1
#ifdef __x86_64__
#if USEAVX2
    const int unroll = 16;
#else
    const int unroll = 8;
#endif
#else
    const int unroll = 8;
#endif
    for (; pos + unroll - 1 < limit; pos += unroll) {

        uint16_t add[unroll]{};

        for (int i = 0; i < unroll; ++i) add[i] = input[i + pos + radius] - input[i + pos - radius - 1];

#ifdef __x86_64__
#if USEAVX2
        __m256i addreg = _mm256_loadu_si256((__m256i_u *) add);
        addreg = _mm256_add_epi16(addreg, _mm256_slli_si256(addreg, 2));
        addreg = _mm256_add_epi16(addreg, _mm256_slli_si256(addreg, 4));
        addreg = _mm256_add_epi16(addreg, _mm256_slli_si256(addreg, 8));
        __m256i from_left = _mm256_set1_epi16(_mm256_extract_epi16(addreg, 7));

        uint16_t mask[16]{};
        memset(mask + 8, -1, 16);
        from_left = _mm256_and_si256(from_left, _mm256_load_si256((__m256i_u *) mask));
        addreg = _mm256_add_epi16(addreg, from_left);
        _mm256_storeu_si256((__m256i_u *) add, addreg);
#else
        __m128i addreg = _mm_loadu_si128((__m128i *) add);
        addreg = _mm_add_epi16(addreg, _mm_slli_si128(addreg, 2));
        addreg = _mm_add_epi16(addreg, _mm_slli_si128(addreg, 4));
        addreg = _mm_add_epi16(addreg, _mm_slli_si128(addreg, 8));
        _mm_storeu_si128((__m128i *) add, addreg);
#endif
#else
        test();
        // basically want it to do this:
        for (int i = 1; i < unroll; ++i) add[i] += add[i - 1];

            // uint16x8_t result = vld1q_u16(add);
            // result = vaddq_u16(result, vextq_u16(result, vdupq_n_s16(0), 1));
            // result = vaddq_u16(result, vextq_u16(result, vdupq_n_s16(0), 2));
            // result = vaddq_u16(result, vextq_u16(result, vdupq_n_s16(0), 4));
            // vst1q_u16(add, result);
#endif

        uint16_t vals[unroll];
        for (int i = 0; i < unroll; ++i) {
            vals[i] = currentSum + add[i];
        }

        for (int i = 0; i < unroll; ++i) output[pos + i] = vals[i];
        currentSum += add[unroll - 1];
    }

    for (; pos < limit; ++pos) {
        currentSum -= input[pos - radius - 1];
        currentSum += input[pos + radius];
        output[pos] = currentSum;
    }

    // 3. special case, executed only if size <= 2*radius + 1
    limit = std::min(radius + 1, size);
    for (; pos < limit; pos++) {
        output[pos] = currentSum;
    }

    // 4. right border - time spend in this loop can be ignored, no need to
    // optimize it
    for (; pos < size; ++pos) {
        currentSum -= input[pos - radius - 1];
        output[pos] = currentSum;
    }
}
