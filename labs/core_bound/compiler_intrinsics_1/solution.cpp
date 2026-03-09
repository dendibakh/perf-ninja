
#include "solution.h"
#include <immintrin.h>
#include <memory>

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
  const uint8_t *substract_ptr = input.data() + pos - radius - 1;
  const uint8_t *add_ptr = input.data() + pos + radius;
  const uint16_t *outputPtr = output.data() + pos;

  __m128i currentSum_m128i = _mm_set1_epi16(currentSum);

  int i = 0;
  for (; i + 8 <= limit - pos; i += 8) {
    __m128i substract_m128i = _mm_loadu_si64(substract_ptr + i);
    __m128i add_m128i = _mm_loadu_si64(add_ptr + i);
    __m128i sub = _mm_cvtepu8_epi16(substract_m128i);
    __m128i add = _mm_cvtepu8_epi16(add_m128i);

    __m128i diff = _mm_sub_epi16(add, sub);

    __m128i s = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    s = _mm_add_epi16(s, _mm_slli_si128(s, 4));
    s = _mm_add_epi16(s, _mm_slli_si128(s, 8));

    __m128i result = _mm_add_epi16(s, currentSum_m128i);
    _mm_storeu_si128((__m128i*)(outputPtr + i), result);

    currentSum = (uint16_t)_mm_extract_epi16(result, 7);
    currentSum_m128i = _mm_set1_epi16(currentSum);
  }
  pos += i;

  // שארית scalar
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
