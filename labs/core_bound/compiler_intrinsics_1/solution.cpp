
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
  auto prefix_scan8_epi32 = [](__m256i v) -> __m256i {
    // Prefix scan for 8x 32-bit ints `v`
    __m256i t = _mm256_slli_si256(v, 4);
    v = _mm256_add_epi32(v, t);
    t = _mm256_slli_si256(v, 8);
    v = _mm256_add_epi32(v, t);
    __m128i lo128 = _mm256_castsi256_si128(v);
    __m128i lo_last = _mm_shuffle_epi32(lo128, _MM_SHUFFLE(3, 3, 3, 3));
    __m256i carry = _mm256_inserti128_si256(_mm256_setzero_si256(), lo_last, 1);
    return _mm256_add_epi32(v, carry);
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

    __m128i dlo = _mm_sub_epi16(nlo, olo); // 8 int16 deltas
    __m128i dhi = _mm_sub_epi16(nhi, ohi); // next 8 int16 deltas

    __m256i d0 = _mm256_cvtepi16_epi32(dlo);
    __m256i s0 = prefix_scan8_epi32(d0);
    __m256i base0 = _mm256_set1_epi32(currentSum);
    __m256i out0_32 = _mm256_add_epi32(s0, base0); // 8 int32 outputs

    int base1_scalar = currentSum + _mm256_extract_epi32(s0, 7);

    __m256i d1 = _mm256_cvtepi16_epi32(dhi);
    __m256i s1 = prefix_scan8_epi32(d1);
    __m256i base1 = _mm256_set1_epi32(base1_scalar);
    __m256i out1_32 = _mm256_add_epi32(s1, base1); // next 8 int32 outputs

    // Pack 2x8 int32 -> 16x uint16 and store in order
    __m128i out0_lo = _mm256_castsi256_si128(out0_32);
    __m128i out0_hi = _mm256_extracti128_si256(out0_32, 1);
    __m128i packed0 = _mm_packus_epi32(out0_lo, out0_hi);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(outPtr + pos), packed0);

    __m128i out1_lo = _mm256_castsi256_si128(out1_32);
    __m128i out1_hi = _mm256_extracti128_si256(out1_32, 1);
    __m128i packed1 = _mm_packus_epi32(out1_lo, out1_hi);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(outPtr + pos + 8), packed1);

    currentSum = base1_scalar + _mm256_extract_epi32(s1, 7);
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
