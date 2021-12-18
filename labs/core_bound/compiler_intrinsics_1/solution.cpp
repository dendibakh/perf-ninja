
#include "solution.h"
#include <memory>

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  int currentSum = 0;
  int size = static_cast<int>(input.size());

  // 1. left border - time spend in this loop can be ignored, no need to optimize it
  for (int i = 0; i < std::min<int>(size, radius); ++i) {
    currentSum += input[i];
  }

  int pos = 0;
  int limit = std::min(radius + 1, size - radius);
  for (pos = 0; pos < limit; ++pos) {
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  // 2. main loop.

  // sum[pos + 0] = sum - input[pos - r - 1] + input[pos + r]
  // 
  // sum[pos + 1] = sum - input[pos - r - 1] - input[pos - r] + input[pos + r] + input[pos + r + 1]
  // 
  // sum[pos + 2] = sum - input[pos - r - 1] - input[pos - r] - input[pos - r + 1] + input[pos + r] + input[pos + r + 1] + input[pos + r + 2]
  //
  // sum[pos + 3] = sum - input[pos - r - 1] -       ...      - input[pos - r + 2] + input[pos + r] +          ...       + input[pos + r + 3]
  //
  // sum[pos + 4] = sum - input[pos - r - 1] -       ...      - input[pos - r + 3] + input[pos + r] +          ...       + input[pos + r + 4]

  __m128i currSum = _mm_set1_epi16(currentSum);

  limit = size - radius;
  const int stepsse = sizeof(__m128i) / sizeof(OutputVector::value_type);

  for (; pos < limit - (stepsse - 1); pos += stepsse) {
    __m128i sub64 = _mm_loadu_si64(reinterpret_cast<__m128i const*>(input.data() + pos - radius - 1));
    __m128i sub = _mm_cvtepu8_epi16(sub64);

    __m128i add64 = _mm_loadu_si64(reinterpret_cast<__m128i const*>(input.data() + pos + radius));
    __m128i add = _mm_cvtepu8_epi16(add64);

    __m128i difference = _mm_sub_epi16(add, sub);

    difference = _mm_add_epi16(difference, _mm_slli_si128(difference, 2)); // 1 * 16 bytes
    difference = _mm_add_epi16(difference, _mm_slli_si128(difference, 4)); // 2 * 16 bytes
    difference = _mm_add_epi16(difference, _mm_slli_si128(difference, 8)); // 4 * 17 bytes
    
    __m128i values = _mm_add_epi16(currSum, difference);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data() + pos), values);

    currSum = _mm_set1_epi16(_mm_extract_epi16(values, 7));
  }

  currentSum = _mm_extract_epi16(currSum, 7);

  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  // 3. special case, executed only if size <= 2*radius + 1
  limit = std::min(radius + 1, size);
  for (; pos < limit; ++pos) {
    output[pos] = currentSum;
  }

  // 4. right border - time spend in this loop can be ignored, no need to optimize it
  for (; pos < size; ++pos) {
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }
}
