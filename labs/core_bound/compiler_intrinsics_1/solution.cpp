
#include <cstdint>
#include <vector>

using InputVector = std::vector<uint8_t>;
using OutputVector = std::vector<uint16_t>;
constexpr uint8_t radius = 13; // assume diameter (2 * radius + 1) to be less
                               // than 256 so results fits in uint16_t

void init(InputVector &data);
void zero(OutputVector &data, std::size_t size);
void imageSmoothing(const InputVector &inA, uint8_t radius,
                    OutputVector &outResult);

#include <algorithm>
#include <atomic>
#include <cstring>
#include <memory>

#ifdef __x86_64__
#include <immintrin.h>
#include <xmmintrin.h>
#else
#include <arm_neon.h>
#endif

#include <array>
#include <iostream>

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

    const int unroll = 16;
    for (; pos + unroll - 1 < limit; pos += unroll) {

        uint16_t add[unroll]{};

        for (int i = 0; i < unroll; ++i) add[i] = input[i + pos + radius] - input[i + pos - radius - 1];

#ifdef __x86_64__
        __m256i addreg = _mm256_loadu_si256((__m256i *) add);
        addreg = _mm256_add_epi16(addreg, _mm256_slli_si256(addreg, 2));
        addreg = _mm256_add_epi16(addreg, _mm256_slli_si256(addreg, 4));
        addreg = _mm256_add_epi16(addreg, _mm256_slli_si256(addreg, 8));
        __m256i from_left = _mm256_set1_epi16(_mm256_extract_epi16(addreg, 7));

        uint16_t mask[16]{};
        memset(mask + 8, -1, 16);
        from_left = _mm256_and_si256(from_left, _mm256_load_si256((__m256i *) mask));
        addreg = _mm256_add_epi16(addreg, from_left);
        _mm256_storeu_si256((__m256i *) add, addreg);
#else
        uint16x8_t result = vcombine_u16(vld1_u16(add), vld1_u16(add + 8));
        result = vaddq_u16(result, vextq_u16 (result, result, 1));
        result = vaddq_u16(result, vextq_u16 (result, result, 2));
        result = vaddq_u16(result, vextq_u16 (result, result, 4));

        // Store the result back to memory
        uint16x4_t lo = vget_low_u16(result);
        uint16x4_t hi = vget_high_u16(result);
        vst1_u16(add, lo);
        vst1_u16(add + 8, hi);
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
