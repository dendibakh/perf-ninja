
#include "solution.h"
#include <memory>
#include <immintrin.h>

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  int pos = 0;
  int currentSum = 0;
  int size = static_cast<int>(input.size());
  const uint8_t *inPtr = input.data();
  uint16_t *outPtr = output.data();

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

#if 1 || (defined(__AVX2__) && defined(__SSE4_1__))
  // Process 16 outputs per iteration using AVX2
  auto prefix_scan8_epi16 = [](__m128i v) -> __m128i {
    // Prefix scan for 8x 16-bit ints `v`
    __m128i t = _mm_slli_si128(v, 2);
    v = _mm_add_epi16(v, t);
    t = _mm_slli_si128(v, 4);
    v = _mm_add_epi16(v, t);
    t = _mm_slli_si128(v, 8);
    return _mm_add_epi16(v, t);
  };

  for (; pos + 16 <= limit; pos += 16) {
    const uint8_t *pNew = inPtr + pos + radius;
    const uint8_t *pOld = inPtr + pos - radius - 1;

    __m128i n = _mm_loadu_si128(reinterpret_cast<const __m128i *>(pNew));
    __m128i o = _mm_loadu_si128(reinterpret_cast<const __m128i *>(pOld));

    __m128i nlo = _mm_cvtepu8_epi16(n);
    __m128i olo = _mm_cvtepu8_epi16(o);
    __m128i nhi = _mm_cvtepu8_epi16(_mm_srli_si128(n, 8));
    __m128i ohi = _mm_cvtepu8_epi16(_mm_srli_si128(o, 8));

    __m128i d0 = _mm_sub_epi16(nlo, olo); // 8 int16 deltas
    __m128i d1 = _mm_sub_epi16(nhi, ohi); // next 8 int16 deltas

    __m128i s0 = prefix_scan8_epi16(d0);
    __m128i base0 = _mm_set1_epi16(currentSum);
    __m128i out0_16 = _mm_add_epi16(s0, base0); // 8 int16 outputs

    int base1_scalar = currentSum + _mm_extract_epi16(s0, 7);

    __m128i s1 = prefix_scan8_epi16(d1);
    __m128i base1 = _mm_set1_epi16(base1_scalar);
    __m128i out1_16 = _mm_add_epi16(s1, base1); // next 8 int16 outputs

    // Store in order
    _mm_storeu_si128(reinterpret_cast<__m128i *>(outPtr + pos), out0_16);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(outPtr + pos + 8), out1_16);

    currentSum = base1_scalar + _mm_extract_epi16(s1, 7);
  }
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
