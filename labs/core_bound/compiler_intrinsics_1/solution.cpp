
#include "solution.h"

#include <emmintrin.h>
#include <immintrin.h>
#include <memory>
#include <cstdio>

void print_epi8(__m128i vec)
{
  uint8_t vals[16];
  _mm_storeu_si128(reinterpret_cast<__m128i*>(vals), vec);
  for(int i = 0; i < 16;i++)
  {
    printf("%u, ", vals[i]);
  }
  printf("\n");
}

void print_epi8(__m256i vec)
{
  uint8_t vals[32];
  _mm256_storeu_si256(reinterpret_cast<__m256i*>(vals), vec);
  for(int i = 0; i < 32;i++)
  {
    printf("%u, ", vals[i]);
  }
  printf("\n");
}

void print_epi16(__m128i vec)
{
  uint16_t vals[8];
  _mm_storeu_si128(reinterpret_cast<__m128i*>(vals), vec);
  for(int i = 0; i < 8;i++)
  {
    printf("%u, ", vals[i]);
  }
  printf("\n");
}


void print_epi16(__m256i vec)
{
  uint16_t vals[16];
  _mm256_storeu_si256(reinterpret_cast<__m256i*>(vals), vec);
  for(int i = 0; i < 16;i++)
  {
    printf("%u, ", vals[i]);
  }
  printf("\n");
}


void print_actual_data(const InputVector& input, int start, int end)
{
  for(int i = start; i <= end; i++)
  {
    printf("%u, ", input[i]);
  }
  printf("\n");
}

__m128i Shift(__m128i input)
{
    __m128i result = _mm_add_epi16(input, _mm_slli_si128(input, 2)); // shift by 1 element
    result = _mm_add_epi16(result, _mm_slli_si128(result, 4)); // shift by 2 elements
    result = _mm_add_epi16(result, _mm_slli_si128(result, 8));// shift by 4 elements

    return result;
}

#define SOLUTION
#ifdef SOLUTION
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

  // 2. main loop. During optimization, focus mainly on this part
  limit = size - radius;
  // SSE solution
  const uint8_t* subtractPtr = input.data() + pos - radius -1;
  const uint8_t* addPtr = input.data() + pos + radius;
  const uint16_t* outputPtr = output.data() + pos;

  // first load

  #define SSE_8
  int i = 0;
  #ifdef SSE_8
  __m128i current = _mm_set1_epi16(currentSum);
  for(; i+7 < limit - pos; i+=8)
  {
    // Calculate vector diff
    __m128i sub_u8 = _mm_loadu_si64(subtractPtr + i);
    __m128i sub = _mm_cvtepu8_epi16(sub_u8); // widing it 

    __m128i add_u8 = _mm_loadu_si64(addPtr + i);
    __m128i add = _mm_cvtepu8_epi16(add_u8);

    __m128i diff = _mm_sub_epi16(add, sub); // order matters, so highest address  first

    // Calculate vector prefix sum for 8 elements: SIMD prefix sum
    __m128i s = _mm_add_epi16(diff, _mm_slli_si128(diff, 2)); // shift by 1 element
    s = _mm_add_epi16(s, _mm_slli_si128(s, 4)); // shift by 2 elements
    s = _mm_add_epi16(s, _mm_slli_si128(s, 8));// shift by 4 elements

    // Store the result
    __m128i result = _mm_add_epi16(s,current);
    _mm_storeu_si128((__m128i*)(outputPtr + i), result);

    // Update current sum 
    currentSum = (uint16_t)_mm_extract_epi16(result,7);
    current = _mm_set1_epi16(currentSum);
  }
  #endif
  #ifdef AVX_16
  __m256i current = _mm256_set1_epi16(currentSum);
  for(; i+15 < limit - pos; i+=16)
  {
    // Calculate vector diff
    __m128i sub_u8 = _mm_loadu_si128((__m128i*)(subtractPtr + i));
    __m256i sub = _mm256_cvtepu8_epi16(sub_u8); // widing it 

    __m128i add_u8 = _mm_loadu_si128((__m128i*)(addPtr + i));;
    __m256i add = _mm256_cvtepu8_epi16(add_u8);

    __m256i diff = _mm256_sub_epi16(add, sub); // order matters, so highest address  first


    // _mm256_slli_si256 is restricted/limited to the lower 128 bits (for whatever fucking reason)
    // which means I have to split them up
     // Calculate vector prefix sum for 16 elements: SIMD prefix sum
    __m128i low = _mm256_extracti128_si256(diff, 0);
    __m128i high = _mm256_extracti128_si256(diff, 1);
    
    __m128i shifted_low = Shift(low);
    __m128i shifted_high = Shift(high);


    // We add the total sum of the lower (found in the last 16 bit value), to the higher vector
    // in order to treat it as a single vector rather than two
    uint16_t lower_total = (uint16_t)_mm_extract_epi16(shifted_low, 7);
    __m128i lower_add = _mm_set1_epi16(lower_total);
    shifted_high = _mm_add_epi16(shifted_high, lower_add);

    // Store the result
    __m256i result = _mm256_castsi128_si256(shifted_low);
    result = _mm256_inserti128_si256(result, shifted_high, 1);
    result = _mm256_add_epi16(result, current);
   
    _mm256_storeu_si256((__m256i*)(outputPtr + i), result);

    // Update current sum 
    currentSum = (uint16_t)_mm256_extract_epi16(result,15);
    
    current = _mm256_set1_epi16(currentSum); 
  }
  #endif
  #ifdef AVX_32
  __m256i current = _mm256_set1_epi16(currentSum);
  for(; i+15 < limit - pos; i+=16)
  {
    // Calculate vector diff
    __m128i sub_u8 = _mm_loadu_si128((__m128i*)(subtractPtr + i));
    __m256i sub = _mm256_cvtepu8_epi16(sub_u8); // widing it 

    __m128i add_u8 = _mm_loadu_si128((__m128i*)(addPtr + i));;
    __m256i add = _mm256_cvtepu8_epi16(add_u8);

    __m256i diff = _mm256_sub_epi16(add, sub); // order matters, so highest address  first

    __m512i extended_diff = _mm512_inserti64x4(_mm512_setzero_si512(), diff, 0);

    // Calculate vector prefix sum for 8 elements: SIMD prefix sum
    __m512i s = _mm512_add_epi16(extended_diff, _mm512_bslli_epi128(extended_diff, 2)); // shift by 1 element
    s = _mm512_add_epi16(s, _mm512_bslli_epi128(s, 4)); // shift by 2 elements
    s = _mm512_add_epi16(s, _mm512_bslli_epi128(s, 8));// shift by 4 elements 
    s = _mm512_add_epi16(s, _mm512_bslli_epi128(s, 16));// shift by 8 elements 

    // Store the result
    __m256i result = _mm512_extracti64x4_epi64(s,0);
    _mm256_storeu_si256((__m256i*)(outputPtr + i), result);

    // Update current sum 
    currentSum = (uint16_t)_mm256_extract_epi16(result,15);
    current = _mm256_set1_epi16(currentSum);
  }
  #endif
  pos += i;

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
#elif BAD_SOLUTION
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
  limit = size - (load_size/2 - 1);
  #pragma clang loop vectorize(enable)
  // this is more than 2 times slower
  for (; pos < limit; ++pos) 
  {
    const uint32_t win_start = pos-radius;
    const uint32_t win_end = pos + radius;
  
    __m256i data= _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&input[win_start]));
  
    __m256i data_low = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(data, 0));
    __m256i data_high = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(data, 1));

    __m256i horizontal_sum16 = _mm256_hadds_epi16(data_low, data_high);
    horizontal_sum16 = _mm256_hadds_epi16(horizontal_sum16, _mm256_setzero_si256());
    horizontal_sum16 = _mm256_hadds_epi16(horizontal_sum16, _mm256_setzero_si256());
    horizontal_sum16 = _mm256_hadds_epi16(horizontal_sum16, _mm256_setzero_si256());

    __m128i hori_low_split = _mm256_extracti128_si256(horizontal_sum16, 0);
    __m128i hori_high_split = _mm256_extracti128_si256(horizontal_sum16, 1);

    __m128i final_result = _mm_add_epi16(hori_low_split, hori_high_split);

    output[pos] = _mm_extract_epi16(final_result, 0);
    output[pos] -= input[win_end + 1];  
/*
    if(pos == 39982)
    {
          printf("Actual data: ");
    print_actual_data(input,win_start, win_end);
    printf("Data low:  ");
    print_epi16(data_low);
    printf("Data high:  ");
    print_epi16(data_high);
    printf("Horizontal_sum:   ");
    print_epi16(horizontal_sum16);
    printf("Final result:   ");
    print_epi16(final_result);
    printf("Output:  %u     start_rest: %u   end_rest: %u \n", output[pos], start_rest, start_rest + diff);
    }

    */
  }

  currentSum = output[pos-1];

  // 3. special case, executed only if size <= 2*radius + 1
  limit = std::min(radius + 1, size);
  #pragma clang loop vectorize(enable)
  for (; pos < limit; pos++) {
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
#else
static void imageSmoothing(const InputVector &input, uint8_t radius,
                               OutputVector &output) {
  int pos = 0;
  int currentSum = 0;
  int size = (int)input.size();

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

  // 2. main loop. During optimization, focus mainly on this part
  limit = size - radius;

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
#endif
