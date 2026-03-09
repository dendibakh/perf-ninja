
#include "solution.h"
#include <immintrin.h>
#include <memory>

static inline __m128i prefix_sum_4_epi32(__m128i v) {
    __m128i t;

    t = _mm_slli_si128(v, 4);   // shift by 1 int (4 bytes)
    v = _mm_add_epi32(v, t);

    t = _mm_slli_si128(v, 8);   // shift by 2 ints
    v = _mm_add_epi32(v, t);

    return v;
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
  for (; pos + 4 <= limit; pos += 4) {

    __m128i entering = _mm_loadu_si128((const __m128i*)(input.data() + pos + radius));
    __m128i leaving  = _mm_loadu_si128((const __m128i*)(input.data() + pos - radius - 1));

    __m128i delta = _mm_sub_epi32(entering, leaving);

    __m128i scan = prefix_sum_4_epi32(delta);

    __m128i base = _mm_set1_epi32(currentSum);
    scan = _mm_add_epi32(scan, base);

    _mm_storeu_si128((__m128i*)(output.data() + pos), scan);

    currentSum = _mm_extract_epi32(scan, 3);
  }

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
