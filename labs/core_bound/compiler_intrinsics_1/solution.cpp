
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

  const auto *inPtr = input.data();
  auto* outPtr = output.data();
  for (; pos <= limit - 8; pos += 8)
  {
    auto sub_vec = _mm_loadu_si64(inPtr + pos - radius - 1);
    auto sub = _mm_cvtepu8_epi16(sub_vec);
    auto add_vec = _mm_loadu_si64(inPtr + pos + radius);
    auto add = _mm_cvtepu8_epi16(add_vec);

    auto diff = _mm_sub_epi16(add, sub);
    auto sum = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    sum = _mm_add_epi16(sum, _mm_slli_si128(sum, 4));
    sum = _mm_add_epi16(sum, _mm_slli_si128(sum, 8));
    auto result = _mm_add_epi16(_mm_set1_epi16(currentSum), sum);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(outPtr + pos), result);
    currentSum = _mm_extract_epi16(result, 7);
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
