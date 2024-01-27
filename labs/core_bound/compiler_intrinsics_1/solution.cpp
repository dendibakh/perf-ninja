
#include "solution.h"
#include <memory>

#include <immintrin.h>

void main_loop_old(const InputVector &input, int limit, int& pos, OutputVector &output, int& currentSum)
{
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }
}

void main_loop_sse(const InputVector &input, int limit, int& pos, OutputVector &output, int& currentSum)
{
  const uint8_t* first = input.data() + pos - radius - 1;
  const uint8_t* last = input.data() + radius + pos;
  uint16_t* out = output.data() + pos;
  __m128i current = _mm_set1_epi16(currentSum);

  int i = 0;
  for (; i+7 < limit - pos; i += 8) {
        __m128i sub = _mm_loadu_si64(first + i);
        __m128i sub2 = _mm_cvtepu8_epi16(sub);
        __m128i add = _mm_loadu_si64(last + i);
        __m128i add2 = _mm_cvtepu8_epi16(add);

        __m128i diff = _mm_sub_epi16(add2, sub2);

        __m128i s = _mm_add_epi16(diff, _mm_slli_si128(diff, 2));
        s = _mm_add_epi16(s, _mm_slli_si128(s, 4));
        s = _mm_add_epi16(s, _mm_slli_si128(s, 8));

        __m128i res = _mm_add_epi16(s, current);
        _mm_storeu_si128((__m128i*)(out + i), res);

        currentSum = (uint16_t)_mm_extract_epi16(res, 7);
        current = _mm_set1_epi16(currentSum);
    }
  pos += i;
}

#define real_mm256_slli_si256(A, N) _mm256_alignr_epi8(A, _mm256_permute2x128_si256(A, A, _MM_SHUFFLE(0, 0, 2, 0)), 16 - N)

void main_loop_avx(const InputVector &input, int limit, int& pos, OutputVector &output, int& currentSum)
{
  const uint8_t* first = input.data() + pos - radius - 1;
  const uint8_t* last = input.data() + radius + pos;
  uint16_t* out = output.data() + pos;
  __m256i current = _mm256_set1_epi16(currentSum);

  int i = 0;
  for (; i+15 < limit - pos; i += 16) {
        __m128i sub =  _mm_loadu_si128((const __m128i_u*)(first + i));
        __m256i sub2 = _mm256_cvtepu8_epi16(sub);
        __m128i add = _mm_loadu_si128((const __m128i_u*)(last + i));
        __m256i add2 = _mm256_cvtepu8_epi16(add);

        __m256i diff = _mm256_sub_epi16(add2, sub2);

        __m256i s = _mm256_add_epi16(diff, real_mm256_slli_si256(diff, 2));
        s = _mm256_add_epi16(s, real_mm256_slli_si256(s, 4));
        s = _mm256_add_epi16(s, real_mm256_slli_si256(s, 8));
        s = _mm256_add_epi16(s, real_mm256_slli_si256(s, 16));

        __m256i res = _mm256_add_epi16(s, current);
        _mm256_storeu_si256((__m256i*)(out + i), res);

        currentSum = (uint16_t)_mm256_extract_epi16(res, 15);
        current = _mm256_set1_epi16(currentSum);
    }
  pos += i;
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
  //main_loop_sse(input, limit, pos, output, currentSum);
  main_loop_avx(input, limit, pos, output, currentSum);
  main_loop_old(input, limit, pos, output, currentSum);

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
