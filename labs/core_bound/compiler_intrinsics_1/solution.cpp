
#include "solution.h"
#include <emmintrin.h>
#include <memory>
#include <immintrin.h>
#include <smmintrin.h>

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
for (; pos + 31 < limit; pos += 32) {
    // ---- Block A (Elements pos to pos+15) ----
    __m256i currentSumVec_A = _mm256_set1_epi16(static_cast<int16_t>(currentSum));

    // ---- Block B (Elements pos+16 to pos+31) ----
    // The following operations for Block B are independent of Block A's result
    // and can be executed in parallel by the CPU.
    __m128i sub_u8_B = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[pos + 16 - radius - 1]));
    __m128i add_u8_B = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[pos + 16 + radius]));

    // ---- Block A (continued) ----
    __m128i sub_u8_A = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[pos - radius - 1]));
    __m128i add_u8_A = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[pos + radius]));

    __m256i sub_A = _mm256_cvtepu8_epi16(sub_u8_A);
    __m256i add_A = _mm256_cvtepu8_epi16(add_u8_A);

    // ---- Block B (continued) ----
    __m256i sub_B = _mm256_cvtepu8_epi16(sub_u8_B);
    __m256i add_B = _mm256_cvtepu8_epi16(add_u8_B);
    
    // ---- Block A (continued) ----
    __m256i diff_A = _mm256_sub_epi16(add_A, sub_A);
    
    // ---- Block B (continued) ----
    __m256i diff_B = _mm256_sub_epi16(add_B, sub_B);

    // Intra-lane prefix sums for both blocks can be interleaved
    __m256i sum_A = _mm256_add_epi16(diff_A, _mm256_slli_si256(diff_A, 2));
    __m256i sum_B = _mm256_add_epi16(diff_B, _mm256_slli_si256(diff_B, 2));
    sum_A = _mm256_add_epi16(sum_A, _mm256_slli_si256(sum_A, 4));
    sum_B = _mm256_add_epi16(sum_B, _mm256_slli_si256(sum_B, 4));
    sum_A = _mm256_add_epi16(sum_A, _mm256_slli_si256(sum_A, 8));
    sum_B = _mm256_add_epi16(sum_B, _mm256_slli_si256(sum_B, 8));

    // --- Finish Block A: This part is on the critical dependency path ---
    int16_t low_lane_total_A = _mm_extract_epi16(_mm256_castsi256_si128(sum_A), 7);
    __m256i bcast_total_A = _mm256_set1_epi16(low_lane_total_A);
    __m256i high_lane_add_A = _mm256_permute2f128_si256(_mm256_setzero_si256(), bcast_total_A, 0x21);
    sum_A = _mm256_add_epi16(sum_A, high_lane_add_A);
    
    __m256i result_A = _mm256_add_epi16(sum_A, currentSumVec_A);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&output[pos]), result_A);

    // --- Dependency Resolved: We now have the starting sum for Block B ---
    int nextCurrentSum = _mm256_extract_epi16(result_A, 15);
    __m256i currentSumVec_B = _mm256_set1_epi16(static_cast<int16_t>(nextCurrentSum));

    // --- Finish Block B: Uses the result from Block A ---
    int16_t low_lane_total_B = _mm_extract_epi16(_mm256_castsi256_si128(sum_B), 7);
    __m256i bcast_total_B = _mm256_set1_epi16(low_lane_total_B);
    __m256i high_lane_add_B = _mm256_permute2f128_si256(_mm256_setzero_si256(), bcast_total_B, 0x21);
    sum_B = _mm256_add_epi16(sum_B, high_lane_add_B);

    __m256i result_B = _mm256_add_epi16(sum_B, currentSumVec_B);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&output[pos + 16]), result_B);
    
    // Update currentSum for the NEXT unrolled iteration
    currentSum = output[pos + 31];
}

  // Remainder
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
