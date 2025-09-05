
#include "solution.h"
#include <emmintrin.h>
#include <memory>
#include <immintrin.h>
#include <smmintrin.h>

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
  for (; pos + 15 < limit; pos += 16) {
    __m256i currentSumVector = _mm256_set1_epi16(currentSum);

    __m128i sub_u8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[pos - radius - 1]));
    __m256i sub = _mm256_cvtepu8_epi16(sub_u8);

    __m128i add_u8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[pos + radius]));
    __m256i add = _mm256_cvtepu8_epi16(add_u8);

    __m256i diff = _mm256_sub_epi16(add, sub);

    __m256i sum = _mm256_add_epi16(diff, _mm256_slli_si256(diff, 2));
    sum = _mm256_add_epi16(sum, _mm256_slli_si256(sum, 4));
    sum = _mm256_add_epi16(sum, _mm256_slli_si256(sum, 8));

    int16_t low_lane_total = _mm256_extract_epi16(sum, 7);
    __m256i bcast_total = _mm256_set1_epi16(low_lane_total);
    __m256i high_lane_add = _mm256_permute2f128_si256(_mm256_setzero_si256(), bcast_total, 0x21);
    sum = _mm256_add_epi16(sum, high_lane_add);

    __m256i result = _mm256_add_epi16(sum, currentSumVector);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&output[pos]), result);
    currentSum = output[pos + 15];
  }

  // Remainder
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
