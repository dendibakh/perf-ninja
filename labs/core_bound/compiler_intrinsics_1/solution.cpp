#include <immintrin.h>
#include "solution.h"
#include <memory>
#include <iostream>
#include <string.h>

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

  // auto print_8 = [](__m128i var) {
  //   uint16_t val[8];
  //   memcpy(val, &var, sizeof(val));
  //   for (int i = 0; i < 8; i++) {
  //     std::cout << (int)val[i] << " ";
  //   }
  //   std::cout << "\n";
  // };
  // auto print_16 = [](__m128i var) {
  //   uint8_t val[16];
  //   memcpy(val, &var, sizeof(val));
  //   for (int i = 0; i < 16; i++) {
  //     std::cout << (int)val[i] << " ";
  //   }
  //   std::cout << "\n";
  // };
  // 2. main loop.
  limit = size - radius;
  static constexpr int kBatch = 16;
  bool debug = true;
  // if (debug) std::cout << currentSum << " " << (int)input[pos - radius - 1] << " " << (int)input[pos + radius] << "\n";
  for (; pos + radius + kBatch < limit; pos += kBatch) {
    __m128i add = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input.data() + pos + radius));
    __m128i sub = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input.data() + pos - radius - 1));

    // if (debug) print_16(add);
    // if (debug) print_16(sub);

    __m128i add_low = _mm_cvtepu8_epi16(add);
    __m128i add_high = _mm_cvtepu8_epi16(_mm_srli_si128(add, 8));
    __m128i sub_low = _mm_cvtepu8_epi16(sub);
    __m128i sub_high = _mm_cvtepu8_epi16(_mm_srli_si128(sub, 8));

    // if (debug) print_8(add_high);
    // if (debug) print_8(sub_high);

    __m128i vec_high = _mm_sub_epi16(add_high, sub_high);
    __m128i vec_low = _mm_sub_epi16(add_low, sub_low);
    // if (debug) print_8(vec_low);
    // if (debug) print_8(vec_high);

    vec_high = _mm_add_epi16(vec_high, _mm_slli_si128(vec_high, 2));
    vec_high = _mm_add_epi16(vec_high, _mm_slli_si128(vec_high, 4));
    vec_high = _mm_add_epi16(vec_high, _mm_slli_si128(vec_high, 8));

    vec_low = _mm_add_epi16(vec_low, _mm_slli_si128(vec_low, 2));
    vec_low = _mm_add_epi16(vec_low, _mm_slli_si128(vec_low, 4));
    vec_low = _mm_add_epi16(vec_low, _mm_slli_si128(vec_low, 8));

    // if (debug) print_8(vec_low);
    // if (debug) print_8(vec_high);

    // if(debug) print_8(vec_low);
    int16_t broadcast_value = _mm_extract_epi16(vec_low, 7);
    __m128i csum_boardcast = _mm_set1_epi16(currentSum);
    __m128i low_bc = _mm_set1_epi16(broadcast_value);
    // if (debug) print_8(low_bc);
    vec_low= _mm_add_epi16(vec_low, csum_boardcast);
    vec_high = _mm_add_epi16(vec_high, csum_boardcast);
    vec_high = _mm_add_epi16(vec_high, low_bc);

    // if (debug) print_8(vec_low);
    // if (debug) print_8(vec_high);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data() + pos), vec_low);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data() + pos + kBatch / 2), vec_high);
    // for (int i = 0; i < kBatch / 2; i++) {
    //   // if (debug) std::cout << output[pos + i] << "\n";
    //   output[pos + i] += currentSum;
    //   // if (debug) std::cout << output[pos + i] << "\n";
    // }
    currentSum = output[pos + kBatch - 1];

    // debug = false;
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
