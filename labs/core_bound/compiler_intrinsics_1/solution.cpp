
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

  for (; pos + 7 < limit; pos += 8) {
      uint16_t add[8]{};

      // This loop calculates the diff for all 8 respective values
      // However currentSum after 2 iterations = currentSum add[0]
      // + add[1]
      for (int i = 0; i < 8; ++i)
        add[i] = input[i + pos + radius] - input[i + pos - radius - 1];

      // Update add so that add[n] contians the sum of all elements between 0..n
      __m128i addreg = _mm_loadu_si128((__m128i*) add);
      addreg = _mm_add_epi16(addreg, _mm_slli_si128(addreg, 2));
      addreg = _mm_add_epi16(addreg, _mm_slli_si128(addreg, 4));
      addreg = _mm_add_epi16(addreg, _mm_slli_si128(addreg, 8));
      _mm_storeu_si128((__m128i*) add, addreg);

      // Vals holds the diffs applied to currentSum
      uint16_t vals[8];
      for (int i = 0; i < 8; ++i) {
        vals[i] = currentSum + add[i];
      }

      // Apply the diffs at their respective locations in output
      for (int i = 0; i < 8; ++i)
        output[pos + i] = vals[i];

      // Update currentSum
      currentSum += add[7];
  }

  for (; pos < limit; ++pos) {
    currentSum += input[pos + radius] - input[pos - radius - 1];
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
