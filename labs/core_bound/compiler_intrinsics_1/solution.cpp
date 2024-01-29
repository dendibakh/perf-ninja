
#include "solution.h"
#include <memory>
#include <immintrin.h>
#include <iostream>

void store_mm256i_as_16bit_zero_extended(__m256i input, uint16_t* output) {
    // Process the lower 32 bytes
    auto low = _mm256_extracti128_si256(input, 0);
    auto extended_low = _mm256_cvtepu8_epi16(low);

    // Process the upper 32 bytes
    auto high = _mm256_extracti128_si256(input, 1);
    auto extended_high = _mm256_cvtepu8_epi16(high);

    // Store the results
    auto mask = _mm256_set1_epi32(-1);

    _mm256_maskstore_epi32((int*)output, mask, extended_low);
    _mm256_maskstore_epi32((int*)(output + 16), mask, extended_low);
}

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
    int N = 8;
    int pos_start = pos;

    for (; pos + N < limit; pos += N) {
        auto r_add = _mm_loadu_si64(&input[pos + radius]);
        auto r_sub = _mm_loadu_si64(&input[pos - radius - 1]);
        auto r_add_ex = _mm_cvtepu8_epi16(r_add);
        auto r_sub_ex = _mm_cvtepu8_epi16(r_sub);

        // --------
        auto result = _mm_sub_epi16(r_add_ex, r_sub_ex);
        auto base = _mm_set1_epi16(currentSum);

        result = _mm_add_epi16(result, _mm_slli_si128(result, 2));
        result = _mm_add_epi16(result, _mm_slli_si128(result, 4));
        result = _mm_add_epi16(result, _mm_slli_si128(result, 8));
        result = _mm_add_epi16(result, base);

        _mm_storeu_si128((__m128i*)&output[pos], result);

        currentSum = _mm_extract_epi16(result, 7);
    }

    for (; pos < limit; pos++) {
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
