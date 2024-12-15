
#include "solution.h"
#include <memory>

#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <emmintrin.h>
#include <immintrin.h>
#endif

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

#if defined(__ARM_NEON) || defined(__aarch64__)
  int i = 0;
  const uint8_t* subPtr = input.data() + pos - radius - 1;
  const uint8_t* addPtr = input.data() + pos + radius;
  const uint16_t* outputPtr = output.data() + pos;

  int16x8_t current = vdupq_n_s16(currentSum);
  int16x8_t mask = vdupq_n_s16(0);

  for (; i + 7 < limit - pos; i += 8) {
    // 1. Calculate vector diff: input[i+radius] - input[i-radius-1]
    uint8x8_t subtract = vld1_u8(subPtr + i);
    uint8x8_t add = vld1_u8(addPtr + i);
    int16x8_t sub_s = vreinterpretq_s16_u16(vmovl_u8(subtract));
    int16x8_t add_s = vreinterpretq_s16_u16(vmovl_u8(add));

    int16x8_t diff = vsubq_s16(add_s, sub_s);

    // 2. Calculate vector prefix sum for 8 elements
    int16x8_t s = diff;
    s = vaddq_s16(s, vextq_s16(mask, s, 7));
    s = vaddq_s16(s, vextq_s16(mask, s, 6));
    s = vaddq_s16(s, vextq_s16(mask, s, 4));

    // 3. Store the result
    int16x8_t result = vaddq_s16(current, s);
    vst1q_s16((int16_t*)(outputPtr + i), result);

    // 4. Broadcast currentSum for the next iteration
    currentSum = (uint16_t) vgetq_lane_s16(result, 7);
    current = vdupq_n_s16(currentSum);
  }
  pos += i;
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
  const uint8_t* subtractPtr = input.data() + pos - radius - 1;
  const uint8_t* addPtr = input.data() + pos + radius;
  const uint16_t* outputPtr = output.data() + pos;
  __m128i current = _mm_set1_epi16(currentSum);

  int i = 0;
  // loop processes 8 elements per iteration
  for (; i + 7 < limit - pos; i += 8) {
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
#endif

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
