#include "solution.h"
#include <immintrin.h>
#include <memory>

template<int byteCount>
__m256i shiftLeft(__m256i source) {
  static_assert(byteCount >= 0 && byteCount <= 16);
  __m256i shiftedRegister = _mm256_permute2x128_si256(source, source, 0x08);
  return _mm256_alignr_epi8(source, shiftedRegister, 16 - byteCount);
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
  const auto* leftRangePtr = input.data() + pos - radius - 1;
  const auto* rightRangePtr = input.data() + pos + radius;
  const auto* outputRangePtr = output.data() + pos;
  const auto iterationCount = limit - pos;
  auto i = 0;
  for (; i + 15 < iterationCount; i += 16) {
    auto leftRange = _mm_loadu_si128((__m128i const*)(leftRangePtr + i));
    auto rightRange = _mm_loadu_si128((__m128i const*)(rightRangePtr + i));
    auto extendedLeftRange = _mm256_cvtepu8_epi16(leftRange);
    auto extendedRightRange = _mm256_cvtepu8_epi16(rightRange);
    auto differenceRange = _mm256_sub_epi16(extendedRightRange, extendedLeftRange);
    auto prefixSumVector = _mm256_add_epi16(differenceRange, shiftLeft<2>(differenceRange));
    prefixSumVector = _mm256_add_epi16(prefixSumVector, shiftLeft<4>(prefixSumVector));
    prefixSumVector = _mm256_add_epi16(prefixSumVector, shiftLeft<8>(prefixSumVector));
    prefixSumVector = _mm256_add_epi16(prefixSumVector, shiftLeft<16>(prefixSumVector));
    auto currentSumVector = _mm256_set1_epi16(static_cast<short>(currentSum));
    auto resultSumVector = _mm256_add_epi16(prefixSumVector, currentSumVector);
    _mm256_storeu_si256((__m256i*)(outputRangePtr + i), resultSumVector);
    currentSum = _mm256_extract_epi16(resultSumVector, 15);
  }

  pos += i;
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
