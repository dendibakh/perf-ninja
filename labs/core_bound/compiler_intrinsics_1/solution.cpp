
#include "solution.h"
#include <memory>
#include <array>
#include <iostream>

#include <x86intrin.h>

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
  constexpr int vec_sz = 8;
  //using vec = int16_t __attribute__((ext_vector_type(vec_sz)));
  limit = size - radius;
  for (; pos + vec_sz <= limit; pos += vec_sz) {
    //vec psum;
    //for (int i = 0; i < vec_sz; ++i) {
    //  psum[i] = (int16_t)input[i + pos + radius] - (int16_t)input[i + pos - radius - 1];
    //}
    //for (int i = 1; i < vec_sz; ++i) {
    //  psum[i] += psum[i - 1];
    //}
    //for (int i = 0; i < vec_sz; ++i) {
    //  psum[i] += currentSum;
    //}
    //for (int i = 0; i < vec_sz; ++i) {
    //  output[pos + i] = psum[i];
    //}
   
    auto d = _mm_sub_epi16(
      _mm_cvtepu8_epi16(_mm_loadu_si64(&input[pos + radius])),
      _mm_cvtepu8_epi16(_mm_loadu_si64(&input[pos - radius - 1]))
    );
    //__m128i d = _mm_loadu_si128((const __m128i*)&psum);
    d = _mm_add_epi16(d, _mm_slli_si128(d, 1 * sizeof(int16_t)));
    d = _mm_add_epi16(d, _mm_slli_si128(d, 2 * sizeof(int16_t)));
    d = _mm_add_epi16(d, _mm_slli_si128(d, 4 * sizeof(int16_t)));
    d = _mm_add_epi16(d, _mm_set1_epi16(currentSum));
    _mm_storeu_si128((__m128i*)&output[pos], d);
    //currentSum = output[pos + vec_sz - 1];
    currentSum = _mm_extract_epi16(d, 7);
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
