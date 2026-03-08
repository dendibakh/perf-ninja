
#include "solution.h"
#include <memory>
#include <immintrin.h>


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
  for (; pos + 16 < limit; pos += 16) {
    // Load 16x uint8 from input (two offsets)
    __m128i a8 = _mm_loadu_si128((__m128i*)&input[pos - radius - 1]);
    __m128i b8 = _mm_loadu_si128((__m128i*)&input[pos + radius]);

    // Widen uint8 → uint16
    __m256i a16 = _mm256_cvtepu8_epi16(a8);
    __m256i b16 = _mm256_cvtepu8_epi16(b8);

    // Load existing output (uint16)
    __m256i out = _mm256_loadu_si256((__m256i*)&output[pos]);

    // out = out - a + b
    out = _mm256_sub_epi16(out, a16);
    out = _mm256_add_epi16(out, b16);

    _mm256_storeu_si256((__m256i*)&output[pos], out);
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
