#include "solution.hpp"

#include <immintrin.h>
#include <cstdio>


#ifdef PRINT
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

void print_binary(unsigned int value)
{
  
  for(unsigned int i= 1 << 31; i > 0; i /= 2)
  {
    if(value & i)
    {
      printf("1, ");
    }
    else
      printf("0, ");
  }
  printf("\n");
}
#endif
// Find the longest line in a file.
// Implementation uses ternary operator with a hope that compiler will
// turn it into a CMOV instruction.
// The code inside the inner loop is equivalent to:
/*
if (s == '\n') {
  longestLine = std::max(curLineLength, longestLine);
  curLineLength = 0;
} else {
  curLineLength++;
}*/


unsigned solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;
  
  //#define SOLUTION

  #ifdef SOLUTION
  unsigned int index = 0;
  for(; index+31 < inputContents.size(); index+= 32)
  {
      __m256i data = _mm256_loadu_si256 ((__m256i*)&inputContents[index]);
      __m256i mask = _mm256_set1_epi8('\n');

      __m256i mask_off = _mm256_cmpeq_epi8(data, mask);

      // Dumping the mask
      unsigned int dumped_mask = _mm256_movemask_epi8(mask_off);
      unsigned int trailing_zeros = 0;
      for(unsigned int shifted_mask = dumped_mask; shifted_mask > 0; shifted_mask >>= trailing_zeros)
      {
        trailing_zeros = _tzcnt_u32(shifted_mask);
        longestLine = std::max(curLineLength + trailing_zeros, longestLine);
        curLineLength = 0;
        shifted_mask >>= 1; // this is to avoid the weird error where a right bitshift by 32 doesnt change the mask,
      }
      curLineLength += _lzcnt_u32(dumped_mask);
  }
  for (;index < inputContents.size(); index++) 
  {
    curLineLength = (inputContents[index] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }
  #else
  for (auto s : inputContents) 
  {
    curLineLength = (s == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }
  #endif
  return longestLine;
}
