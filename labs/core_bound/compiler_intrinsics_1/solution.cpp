
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
  for (; pos + 7 < limit; pos += 8) {
    __m128i currentSumVector = _mm_set1_epi16(currentSum);
    __m128i subU8 = _mm_loadu_si64(&input[pos - radius - 1]);
    __m128i sub = _mm_cvtepu8_epi16(subU8);

    __m128i addU8 = _mm_loadu_si64(&input[pos + radius]);
    __m128i add = _mm_cvtepu8_epi16(addU8);

    __m128i diff = _mm_sub_epi16(add, sub);

    __m128i sum = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    sum = _mm_add_epi16(sum, _mm_slli_si128(sum, 4));
    sum = _mm_add_epi16(sum, _mm_slli_si128(sum, 8));

    __m128i result = _mm_add_epi16(sum, currentSumVector);
    _mm_storeu_si128(reinterpret_cast<__m128i_u *>(&output[pos]), result);
    currentSum = _mm_extract_epi16(result, 7);
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
