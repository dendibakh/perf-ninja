
#include "solution.h"

#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>

__m256i add_constant_to_upper_half(__m256i reg, int16_t constant) {
  // Extract upper 128 bits (8 numbers)
  __m128i upper = _mm256_extracti128_si256(reg, 1);

  // Create constant vector
  __m128i const_vec = _mm_set1_epi16(constant);

  // Add constant to upper 8 numbers
  upper = _mm_add_epi16(upper, const_vec);

  // Insert back into upper half
  return _mm256_inserti128_si256(reg, upper, 1);
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
  const uint8_t *subtractPtr = input.data() + pos - radius - 1;
  const uint8_t *addPtr = input.data() + pos + radius;
  const uint16_t *outputPtr = output.data() + pos;

  // Broadcast 16-bit integer a to all all elements of dst.
  __m256i current = _mm256_set1_epi16(currentSum);

  int i = 0;
  for (; i + 15 < limit - pos; i += 16) {
    // Load unaligned 64-bit integer from memory into the first element of dst.
    __m128i sub_u8 = _mm_loadu_si128((__m128i *)(subtractPtr + i));
    __m256i sub = _mm256_cvtepu8_epi16(sub_u8);
    __m128i add_u8 = _mm_loadu_si128((__m128i *)(addPtr + i));
    __m256i add = _mm256_cvtepu8_epi16(add_u8);

    __m256i diff = _mm256_sub_epi16(add, sub);

    // 2. Calculate vector prefix sum for 16 elements
    __m256i s = _mm256_add_epi16(diff, _mm256_slli_si256(diff, 2));
    s = _mm256_add_epi16(s, _mm256_slli_si256(s, 4));
    s = _mm256_add_epi16(s, _mm256_slli_si256(s, 8));
    s = add_constant_to_upper_half(s, ((uint16_t *)&s)[7]);

    // 3. Store the result
    __m256i result = _mm256_add_epi16(s, current);
    _mm256_storeu_si256((__m256i *)(outputPtr + i), result);

    // 4. Broadcast currentSum for the next iteration
    currentSum = (uint16_t)_mm256_extract_epi16(result, 15);
    current = _mm256_set1_epi16(currentSum);
  }
  pos += i;

  // Still keep the sequential loop to process the remainder.
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
