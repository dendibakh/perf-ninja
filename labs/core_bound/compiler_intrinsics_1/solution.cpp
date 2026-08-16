
#include "solution.h"
#include <immintrin.h>
#include <memory>

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  int pos = 0;
  uint16_t currentSum = 0;
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

  for (; pos + 8 < limit; pos += 8) {
    __m128i fst = _mm_loadu_si128((__m128i*)&input[pos + radius]);
    __m128i sec = _mm_loadu_si128((__m128i*)&input[pos - radius - 1]);
    __m128i res1 = _mm_sub_epi16(fst, sec);
    __m128i shift1 = _mm_slli_si128(res1, 2);
    __m128i res2 = _mm_add_epi16(res1, shift1);
    __m128i shift2 = _mm_slli_si128(res2, 4);
    __m128i res3 = _mm_add_epi16(res2, shift2);
    __m128i shift3 = _mm_slli_si128(res3, 8);

    __m128i res4 = _mm_add_epi16(res3, shift3);

    __m128i consts = _mm_set1_epi16(currentSum);

    __m128i final_res = _mm_add_epi16(consts, res4);

    _mm_storeu_si128((__m128i*)&output[pos], final_res);
    currentSum = output[pos + 7];
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
