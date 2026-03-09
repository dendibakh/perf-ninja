
#include "solution.h"
#include <immintrin.h>
#include <memory>

static inline __m128i prefix_sum_4_epi32(__m128i v) {
    __m128i t;

    // shift by 1 int (4 bytes)
    t = _mm_slli_si128(v, 4);
    v = _mm_add_epi32(v, t);

    // shift by 2 ints (8 bytes)
    t = _mm_slli_si128(v, 8);
    v = _mm_add_epi32(v, t);

    return v;
}

static inline __m128i load_u8x4_to_i32(const uint8_t* ptr) {
    return _mm_setr_epi32(
        static_cast<int>(ptr[0]),
        static_cast<int>(ptr[1]),
        static_cast<int>(ptr[2]),
        static_cast<int>(ptr[3])
    );
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
  for (; pos + 4 <= limit; pos += 4) {
    __m128i entering = load_u8x4_to_i32(input.data() + pos + radius);
    __m128i leaving  = load_u8x4_to_i32(input.data() + pos - radius - 1);

    __m128i delta = _mm_sub_epi32(entering, leaving);
    __m128i scan  = prefix_sum_4_epi32(delta);

    __m128i base = _mm_set1_epi32(static_cast<int>(currentSum));
    __m128i output_block = _mm_add_epi32(scan, base);

    alignas(16) int tmp[4];
    _mm_store_si128(reinterpret_cast<__m128i*>(tmp), output_block);

    output[pos + 0] = static_cast<uint16_t>(tmp[0]);
    output[pos + 1] = static_cast<uint16_t>(tmp[1]);
    output[pos + 2] = static_cast<uint16_t>(tmp[2]);
    output[pos + 3] = static_cast<uint16_t>(tmp[3]);

    currentSum = static_cast<uint32_t>(tmp[3]);
  }

  // שארית scalar
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  currentSum = output[pos - 1];
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
