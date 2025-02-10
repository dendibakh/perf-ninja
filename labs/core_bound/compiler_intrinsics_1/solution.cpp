
#include "solution.h"
#include <memory>
#include <emmintrin.h>
#include <immintrin.h>
#include <iostream>

static int accumulate(uint16_t *output, int currentSum) {
  // init all elements of current to currentSum
  __m128i current{_mm_set1_epi16(currentSum)};

	// Load 4 uint16 to add
	__m128i add16{_mm_loadu_si128((__m128i const*)output)};

	// add prefix sum to current
	add16 = _mm_add_epi16(current, add16);

	// store
	_mm_storeu_si128((__m128i *)output, add16);

	// extract final element for next iter
	return _mm_extract_epi16(add16, 7);
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
  
  for (; pos + 15 < limit; pos += 16) {
	  // Load 16 uint8 to add and sub
	  __m128i add8{_mm_loadu_si128((__m128i const*)&input[pos + radius])};
	  __m128i sub8{_mm_loadu_si128((__m128i const*)&input[pos - radius - 1])};

          // convert to uint16
	  __m256i add16{_mm256_cvtepu8_epi16(add8)};
	  __m256i sub16{_mm256_cvtepu8_epi16(sub8)};

	  // sub subs from adds
	  __m256i diff{_mm256_sub_epi16(add16, sub16)};

	  // prefix sum (add after shifting 1, 2, 4 elements)
	  diff = _mm256_add_epi16(diff, _mm256_slli_si256(diff, 2));
	  diff = _mm256_add_epi16(diff, _mm256_slli_si256(diff, 4));
	  diff = _mm256_add_epi16(diff, _mm256_slli_si256(diff, 8));

	// store
	_mm256_storeu_si256((__m256i *)&output[pos], diff);

	// accumulate to compensate for lack of shift between two 128 bit
	// sections
	currentSum = accumulate(&output[pos], currentSum);
	currentSum = accumulate(&output[pos + 8], currentSum);
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
