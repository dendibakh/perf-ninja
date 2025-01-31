
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

    
    const uint32_t start_rest = win_end + 1;

    for(uint32_t sub_pos = start_rest; sub_pos < start_rest + diff; sub_pos++)
    {
      output[pos] -= input[sub_pos];
    }
    

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
