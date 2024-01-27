#include "solution.hpp"
#include <iostream>

#include <immintrin.h>
#include <bit>

// Find the longest line in a file.

size_t get_longest_from_tail(const std::string &inputContents, size_t start, size_t curLineLength, size_t longestLine) {
  for (; start < inputContents.size(); ++start) {
    curLineLength = (inputContents[start] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }
  return longestLine;
}

template<int max_len>
void extract_zeroes(unsigned src, size_t& curLineLength, size_t& longestLine) {
  unsigned short max_z = max_len;
  while (true) {
    auto zeroes = _tzcnt_u32(src) + 1;
    if (max_z < zeroes) {
      curLineLength += (max_z);
      break;
    }
    curLineLength += zeroes;
    longestLine = std::max(curLineLength - 1, longestLine);
    curLineLength = 0;
    max_z -= zeroes;
    src = src >> zeroes;
  }
}

namespace SSE {
int apply_endl_mask(const __m128i_u* start) {
  auto n_mask = _mm_set1_epi8('\n');
  auto line =  _mm_loadu_si128(start);
  auto res_comp = _mm_cmpeq_epi8(line, n_mask);
  return _mm_movemask_epi8(res_comp);
}

size_t get_longest_line(const std::string &inputContents) {
  size_t longestLine = 0;
  size_t curLineLength = 0;
  const char* start = inputContents.data();
  size_t i = 0;
  for (; i+15 < inputContents.size(); i += 16) {
    auto v = apply_endl_mask((const __m128i_u*)(start + i));
    extract_zeroes<16>(v, curLineLength, longestLine);
  }
  return get_longest_from_tail(inputContents, i, curLineLength, longestLine);
}
}

namespace AVX {
int apply_endl_mask(const __m256i_u* start) {
  auto n_mask = _mm256_set1_epi8('\n');
  auto line =  _mm256_loadu_si256(start);
  auto res_comp = _mm256_cmpeq_epi8(line, n_mask);
  return _mm256_movemask_epi8(res_comp);
}

size_t get_longest_line(const std::string &inputContents) {
  size_t longestLine = 0;
  size_t curLineLength = 0;
  const char* start = inputContents.data();
  size_t i = 0;
  for (; i+31 < inputContents.size(); i += 32) {
    auto v = apply_endl_mask((const __m256i_u*)(start + i));
    extract_zeroes<32>(v, curLineLength, longestLine);
  }
  return get_longest_from_tail(inputContents, i, curLineLength, longestLine);
}
}

unsigned solution(const std::string &inputContents) {
  //return AVX::get_longest_line(inputContents);
  return SSE::get_longest_line(inputContents);
  //return get_longest_from_tail(inputContents, 0, 0, 0); //base
}