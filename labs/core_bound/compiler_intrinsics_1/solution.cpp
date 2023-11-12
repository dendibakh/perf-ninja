
#include "solution.h"
#include <memory>

#if defined(__aarch64__) || defined(_M_ARM64)
#define USE_NEON
#include <arm_neon.h>
#else
#include <immintrin.h>
#endif

namespace {

uint16_t smooth_main_scalar(uint16_t currentSum, int* currentPos, const InputVector& input, uint8_t radius, OutputVector& output) {
  int limit = input.size() - radius;

  int pos =  *currentPos;
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }
  *currentPos = pos;
  return currentSum;
}

#ifdef USE_NEON
uint16_t smooth_main_a64_neon(uint16_t currentSum, int* currentPos, const InputVector& input, uint8_t radius, OutputVector& output) {
  constexpr int step = 8;
  int limit = input.size() - radius;
  limit -= limit % step;

  int pos = *currentPos;

  auto *lhsPtr = input.data() + pos - radius - 1;
  auto *rhsPtr = input.data() + pos + radius;
  auto *outPtr = reinterpret_cast<uint16x8_t *>(output.data() + pos);

  uint16x8_t currentSumVec = vdupq_n_u16(currentSum);

  for (auto p = pos; p < limit; p += step) {
    auto lhs = *reinterpret_cast<const uint8x8_t *>(lhsPtr);
    auto rhs = *reinterpret_cast<const uint8x8_t *>(rhsPtr);
    auto diff = vsubl_u8(rhs, lhs);
    auto zero = vdupq_n_u16(0);

    diff = diff + vextq_u16(zero, diff, 7);
    diff = diff + vextq_u16(zero, diff, 6);
    diff = diff + vextq_u16(zero, diff, 4);
    currentSumVec += diff;
    *outPtr = currentSumVec;
    currentSumVec = vdupq_laneq_u16(currentSumVec, 7);

    lhsPtr+=step;
    rhsPtr+=step;
    outPtr++;
    pos += step;
  }
  
  currentSum = vgetq_lane_u16(currentSumVec, 0);
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1] + input[pos + radius];
    output[pos] = currentSum;
  }
  *currentPos = pos;
  return currentSum;
}
#endif
#if defined(__SSE3__)
uint16_t smooth_main_x86_sse(uint16_t currentSum, int* currentPos, const InputVector& input, uint8_t radius, OutputVector& output) {
  constexpr int step = 8;
  int limit = input.size() - radius;
  limit -= limit % step;

  int pos = *currentPos;

  auto *lhsPtr = input.data() + pos - radius - 1;
  auto *rhsPtr = input.data() + pos + radius;
  auto *outPtr = reinterpret_cast<__m128i_u *>(output.data() + pos);

  auto currentSumVec = _mm_set1_epi16(currentSum);

  for (auto p = pos; p < limit; p += step) {
    auto lhs = _mm_cvtepu8_epi16(_mm_loadu_si64(lhsPtr));
    auto rhs = _mm_cvtepu8_epi16(_mm_loadu_si64(rhsPtr));
    auto diff = _mm_sub_epi16(rhs, lhs);

    diff = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
    diff = _mm_add_epi16(diff, _mm_slli_si128(diff, 4));
    diff = _mm_add_epi16(diff, _mm_slli_si128(diff, 8));
    currentSumVec = _mm_add_epi16(currentSumVec, diff);
    *outPtr = currentSumVec;
    currentSumVec = _mm_set1_epi16(_mm_extract_epi16(currentSumVec, 7));

    lhsPtr+=step;
    rhsPtr+=step;
    outPtr++;
    pos += step;
  }
  
  currentSum = _mm_extract_epi16(currentSumVec, 0);
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1] + input[pos + radius];
    output[pos] = currentSum;
  }
  *currentPos = pos;
  return currentSum;
}
#endif
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
#ifdef USE_NEON
  currentSum = smooth_main_a64_neon(currentSum, &pos, input, radius, output);
#elif defined(__SSE3__)
  currentSum = smooth_main_x86_sse(currentSum, &pos, input, radius, output);
#else
  currentSum = smooth_main_scalar(currentSum, &pos, input, radius, output);
#endif

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
