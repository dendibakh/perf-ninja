
#include "solution.h"

#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <memory>
#include <ostream>

// Generates a mask where lanes < limit are 0xFFFFFFFF, and lanes >= limit are
// 0x0
__m256i generate_limit_mask(int limit) {
  __m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
  __m256i limit_vec = _mm256_set1_epi32(limit);
  return _mm256_cmpgt_epi32(limit_vec, indices);
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
  for (; pos + 8 <= limit; pos += 8) {
    __m128i sub = _mm_cvtepu8_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&input[pos - radius - 1])));
    __m128i add = _mm_cvtepu8_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&input[pos + radius])));

    __m128i sumVec = _mm_sub_epi16(add, sub);
    sumVec = _mm_add_epi16(sumVec, _mm_setr_epi16(currentSum, 0, 0, 0, 0, 0, 0, 0));

    static constexpr int bytesPerLane = 2;
    sumVec = _mm_add_epi16(sumVec, _mm_bslli_si128(sumVec, bytesPerLane));
    sumVec = _mm_add_epi16(sumVec, _mm_bslli_si128(sumVec, bytesPerLane * 2));
    sumVec = _mm_add_epi16(sumVec, _mm_bslli_si128(sumVec, bytesPerLane * 4));

    _mm_storeu_si128(reinterpret_cast<__m128i*>(&output[pos]), sumVec);
    currentSum = _mm_extract_epi16(sumVec, 7);
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
