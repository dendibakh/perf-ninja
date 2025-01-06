
#include "solution.h"
#include <memory>

#define SOLUTION
#if defined(SOLUTION) && defined(__AVX2__)

#include <immintrin.h>

// #define DEBUG_SHIFT
#ifdef DEBUG_SHIFT
#include <iostream>
#endif

template <int num>
__attribute__((always_inline)) __m256i shiftl16_256(const __m256i a)
{

#ifdef SOL_SSE_SHIFT
  // a: [15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0]

  // low: [15 14 13 12 11 10 9 8]
  __m128i lo = _mm256_extractf128_si256(a, 0);

  // high: [7 6 5 4 3 2 1 0]
  __m128i hi = _mm256_extractf128_si256(a, 1);

  constexpr int bytes = num * 2; // 2 bytes per 16 bit num

  // Shift high and low part. Note that we are still missing the numbers from low that shift to high
  __m128i lo_shift = _mm_slli_si128(lo, bytes);
  __m128i hi_shift = _mm_slli_si128(hi, bytes);

  // We need to shift right in lo to remove the ones that are not moving
  constexpr int lane_width_sse = 128 / 16;
  constexpr int num_r = lane_width_sse - num;
  constexpr int bytes_r = num_r * 2; // 2 bytes per 16 bit num

  // Highest part of low was pushed to high
  __m128i lo_to_merge = _mm_srli_si128(lo, bytes_r);
  __m128i hi_complete = _mm_add_epi16(hi_shift, lo_to_merge);

  return _mm256_set_m128i(hi_complete, lo_shift);

#else

  constexpr int bytes = num * 2;                 // 2 bytes per 16 bit num
  __m256i shifted = _mm256_slli_si256(a, bytes); // we are missing the values that shifted from low to high

  // We need to shift right in lo to remove the ones that are not moving
  constexpr int lane_width_sse = 128 / 16;
  constexpr int num_r = lane_width_sse - num;
  constexpr int bytes_r = num_r * 2; // 2 bytes per 16 bit num

  // Highest part of low was pushed to high
  __m256i lo_to_merge_shift = _mm256_srli_si256(a, bytes_r); // still includes parts of high
  constexpr int permute_mask_lo = 1 << 3;
  // permute_mask_lo[3] == 1 => zero out low part
  // permute_mask_lo[4..7] == 0 => copy low to high
  __m256i lo_to_merge = _mm256_permute2f128_si256(lo_to_merge_shift, lo_to_merge_shift, permute_mask_lo);

  return _mm256_add_epi16(shifted, lo_to_merge);
#endif
}
#endif

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output)
{
  int pos = 0;
  int currentSum = 0;
  int size = static_cast<int>(input.size());

  // 1. left border - time spend in this loop can be ignored, no need to
  // optimize it
  for (int i = 0; i < std::min<int>(size, radius); ++i)
  {
    currentSum += input[i];
  }

  int limit = std::min(radius + 1, size - radius);
  for (pos = 0; pos < limit; ++pos)
  {
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

#if defined(SOLUTION) && defined(__AVX2__)

  // 2. main loop.
  limit = size - radius;

  const uint8_t *pLow = input.data() + pos - radius - 1;
  const uint8_t *pHigh = input.data() + pos + radius;
  const uint16_t *pOut = output.data() + pos;
  __m256i current = _mm256_set1_epi16(currentSum);
  __m256i result;
  constexpr int lane_width_avx = 256 / 16;
  constexpr int lane_width_sse = 128 / 16;

#ifdef DEBUG_SHIFT
  {
    uint8_t testIn[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint16_t res[16] = {};

    std::cout << "In" << std::endl;
    for (auto i = 0; i < 16; ++i)
    {
      std::cout << (int)testIn[i] << ", ";
    }
    std::cout << std::endl;

    __m128i a_u8 = _mm_loadu_si128((__m128i *)testIn); // We loaded 16 uint8
    __m256i a = _mm256_cvtepu8_epi16(a_u8);

    __m256i shifted1 = shiftl16_256<1>(a);
    _mm256_storeu_si256((__m256i *)res, shifted1);
    std::cout << "shifted1" << std::endl;
    for (auto i = 0; i < 16; ++i)
    {
      std::cout << res[i] << ", ";
    }
    std::cout << std::endl;

    __m256i shifted2 = shiftl16_256<2>(a);
    _mm256_storeu_si256((__m256i *)res, shifted2);
    std::cout << "shifted2" << std::endl;
    for (auto i = 0; i < 16; ++i)
    {
      std::cout << res[i] << ", ";
    }
    std::cout << std::endl;

    __m256i shifted4 = shiftl16_256<4>(a);
    _mm256_storeu_si256((__m256i *)res, shifted4);
    std::cout << "shifted4" << std::endl;
    for (auto i = 0; i < 16; ++i)
    {
      std::cout << res[i] << ", ";
    }
    std::cout << std::endl;

    __m256i shifted8 = shiftl16_256<8>(a);
    _mm256_storeu_si256((__m256i *)res, shifted8);
    std::cout << "shifted8" << std::endl;
    for (auto i = 0; i < 16; ++i)
    {
      std::cout << res[i] << ", ";
    }
    std::cout << std::endl;
  }
#endif

  int i = 0;
  for (; i + lane_width_avx - 1 < limit - pos; i += lane_width_avx)
  {
    __m128i sub_u8 = _mm_loadu_si128((__m128i *)(pLow + i)); // We loaded 16 uint8
    __m256i sub = _mm256_cvtepu8_epi16(sub_u8);              // Take the 16 uint8 and convert them to 16 uint16 (this fits in 256 bits)

    __m128i add_u8 = _mm_loadu_si128((__m128i *)(pHigh + i)); // We loaded 16 uint8
    __m256i add = _mm256_cvtepu8_epi16(add_u8);               // Take the 16 uint8 and convert them to 16 uint16 (this fits in 256 bits)

    __m256i diff = _mm256_sub_epi16(add, sub); // Calculate moving window's diffs

    // diff: [15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0]

    // Do prefix sum
    // sums: [15+14 14+13 13+12 12+11 ... 3+2 2+1 1+0 0]
    __m256i sums = _mm256_add_epi16(diff, shiftl16_256<1>(diff));

    // sums: [15+14+13+12 14+13+12+11 ... 3+2+1+0 2+1+0 1+0 0]
    sums = _mm256_add_epi16(sums, shiftl16_256<2>(sums));

    // ...
    sums = _mm256_add_epi16(sums, shiftl16_256<4>(sums));

    // ...
    sums = _mm256_add_epi16(sums, shiftl16_256<8>(sums));

    // Save results
    result = _mm256_add_epi16(current, sums);
    _mm256_storeu_si256((__m256i *)(pOut + i), result);

// Prepare next iteration

// #define OLD_SHUFFLE
#ifdef OLD_SHUFFLE
    currentSum = _mm256_extract_epi16(result, 15);
    current128 = _mm_set1_epi16(currentSum);
    current = _mm256_loadu2_m128i(&current128, &current128);
#else

    // We want to broadcast the last uint16 to all positions
    constexpr int permute_mask_hi = 3 | (1 << 4);
    // permute_mask_hi[0..3] == 3 ==> Copy high to low
    // permute_mask_hi[4..7] == 1 ==> Copy high to high
    __m256i hi_copied_low = _mm256_permute2f128_si256(result, result, permute_mask_hi);
    current = _mm256_srli_si256(hi_copied_low, 7 * 2); // highest part lowered to lowest part, still need to fill the other 7 per 128 lane
    current = _mm256_or_si256(current, _mm256_slli_si256(current, 1 * 2)); // Fill the lowest 32 bits per 128 lane
    current = _mm256_shuffle_epi32(current, 0); // Copy the lowest 32 bits per 128 lane everywhere
#endif
  }
  currentSum = _mm256_extract_epi16(result, 15);
  pos += i;

  // load data unaligned
  for (; pos < limit; ++pos)
  {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

#else

  // 2. main loop.
  limit = size - radius;
  for (; pos < limit; ++pos)
  {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

#endif

  // 3. special case, executed only if size <= 2*radius + 1
  limit = std::min(radius + 1, size);
  for (; pos < limit; pos++)
  {
    output[pos] = currentSum;
  }

  // 4. right border - time spend in this loop can be ignored, no need to
  // optimize it
  for (; pos < size; ++pos)
  {
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }
}
