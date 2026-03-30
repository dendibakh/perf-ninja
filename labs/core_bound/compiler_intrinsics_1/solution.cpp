
#include "solution.h"
#include <memory>
#include "immintrin.h"
#include "emmintrin.h"

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
  for (; pos <= limit - 8; pos += 8) {
    auto sum8 = _mm_setzero_si128();
    for(int j = -radius; j <= radius; j++) {
      auto loadIndex = pos + j;
      const auto *ptr = input.data() + loadIndex;
      auto raw_val = _mm_loadu_si64(ptr);
      auto val = _mm_cvtepu8_epi16(raw_val);
      sum8 = _mm_add_epi16(sum8, val);
    }
    auto *outPtr = reinterpret_cast<__m128i *>(output.data() + pos);
    _mm_storeu_si128(outPtr, sum8);
  }

  currentSum = 0;
  for (int i = pos - 1 - radius; i <= pos - 1 + radius; ++i) {
      currentSum += input[i];
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
