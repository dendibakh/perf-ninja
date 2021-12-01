
#include "solution.h"
#include <memory>
#ifdef __x86_64__
  #include <immintrin.h>
#else
  #include <arm_neon.h>
#endif

constexpr int UNROLL = 8;

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
#ifdef __x86_64__
  // SSE4.1 solution
  const uint8_t* subtractPtr = input.data() + pos - radius - 1;
  const uint8_t* addPtr = input.data() + pos + radius;
  const uint16_t* outputPtr = output.data() + pos;
  __m128i current = _mm_set1_epi16(currentSum);

  int i = 0;
  // loop processes 8 elements per iteration
  for (; i + UNROLL - 1 < limit - pos; i += UNROLL) {
    // 1. Calculate vector diff: input[i+radius] - input[i-radius-1]
    __m128i sub_u8 = _mm_loadu_si64(subtractPtr + i);
    __m128i sub = _mm_cvtepu8_epi16(sub_u8);
    __m128i add_u8 = _mm_loadu_si64(addPtr + i);
    __m128i add = _mm_cvtepu8_epi16(add_u8);

    __m128i diff = _mm_sub_epi16(add, sub);

    // 2. Calculate vector prefix sum for 8 elements
    __m128i s = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    s = _mm_add_epi16(s, _mm_slli_si128(s, 4));
    s = _mm_add_epi16(s, _mm_slli_si128(s, 8));

    // 3. Store the result
    __m128i result = _mm_add_epi16(s, current);
    _mm_storeu_si128((__m128i*)(outputPtr + i), result);

    // 4. Broadcast currentSum for the next iteration
    currentSum = (uint16_t)_mm_extract_epi16(result, 7);
    current = _mm_set1_epi16(currentSum);
  }
  pos += i;
#else
  // ARM solution by Jonathan Hallström
  for (; pos + UNROLL - 1 < limit; pos += UNROLL) {

    uint16_t add[UNROLL]{};

    for (int i = 0; i < UNROLL; ++i)
      add[i] = input[i + pos + radius] - input[i + pos - radius - 1];

    // basically want it to do this:
    // for (int i = 1; i < UNROLL; ++i) 
    //   add[i] += add[i - 1];

    uint16x8_t result = vld1q_u16(add);
    result = vaddq_u16(result, vextq_u16(vdupq_n_s16(0), result, 8 - 1));
    result = vaddq_u16(result, vextq_u16(vdupq_n_s16(0), result, 8 - 2));
    result = vaddq_u16(result, vextq_u16(vdupq_n_s16(0), result, 8 - 4));
    vst1q_u16(add, result);

    uint16_t vals[UNROLL];
    for (int i = 0; i < UNROLL; ++i)
        vals[i] = currentSum + add[i];

    for (int i = 0; i < UNROLL; ++i) 
      output[pos + i] = vals[i];

    currentSum += add[UNROLL - 1];
  }
#endif
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
