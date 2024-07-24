#include <iostream>
#include <immintrin.h>
#include <bitset>
#include "solution.h"

struct Precalc {
  alignas(64) int permutation[256][16]{0};

  constexpr Precalc() {
    for (int m = 0; m < 256; m++) {
      int id = 0;
      for (int i = 0; i < 4; i++) {
        if ((m & (1 << i))) {
          permutation[m][id++] = 2 * i;
          permutation[m][id++] = 2 * i + 1;
        }
      }
      id = 8;
      for (int i = 0; i < 4; i++) {
        if ((m & (1 << (4 + i)))) {
          permutation[m][id++] = 2 * i;
          permutation[m][id++] = 2 * i + 1;
        }
      }
    }
  }  
};

constexpr Precalc pre;

template<typename T = uint32_t, typename U = __m256i>
void print(U x) {
  char* ptr = (char*)&x;
  for (int i = 0; i + sizeof(T) - 1 < sizeof(U); i += sizeof(T)) {
    std::cout << *(T*)(ptr + i) << " ";
  }
  std::cout << std::endl;
}

template<typename T = uint32_t, typename U = __m256i>
void print_bits(U x) {
  char* ptr = (char*)&x;
  for (int i = 0; i + sizeof(T) - 1 < sizeof(U); i += sizeof(T)) {
    std::cout << (std::bitset<8 * sizeof(T)>)*(T*)(ptr + i) << "(" << *(T*)(ptr + i) << ")" << " ";
  }
  std::cout << std::endl;
}

inline __m256i _mm256_cmpgt_epu32(__m256i a, __m256i b) {
  static const __m256i offset = _mm256_set1_epi32(0x80000000);
  auto as = _mm256_add_epi32(a, offset);
  auto bs = _mm256_add_epi32(b, offset);
  // print_bits<int>(bs);
  // print_bits<int>(as);
  return _mm256_cmpgt_epi32(as, bs);
}

inline __m256i less_or_equal(__m256i a, __m256i b) {
  return _mm256_cmpgt_epu32(b, a);
}
// Select items which have S.first in range [lower..upper]
std::size_t select(std::array<S, N> &output, const std::array<S, N> &input,
                   const std::uint32_t lower, const std::uint32_t upper) {
  std::size_t count = 0;
  static const __m256i indices = _mm256_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14);
  static const __m256i mlower = _mm256_set1_epi32(lower);
  static const __m256i mupper = _mm256_set1_epi32(upper);
  int k = 0;

  for (int i = 0; i + 7 < N; i += 8) {

    // for (int j = 0; j < 8; j++) {
    //   std::cout << input[i + j].first << " " << input[i + j].second << " ";
    // }
    // std::cout << std::endl;

    auto x = _mm256_i32gather_epi32((const uint32_t*)&input[i], indices, 4);
    // print(x);
    auto mmask = less_or_equal(mlower, x);
    // print(mmask);
    mmask = _mm256_and_si256(mmask, less_or_equal(x, mupper));


    auto mask = _mm256_movemask_ps(mmask);


    // auto pick_l_indices = _mm_load_si128((__m128i*)&pre.permutation[mask]);
    // auto pick_h_indices = _mm_load_si128((__m128i*)(&pre.permutation[mask][4]));
    // auto out_low = _mm256_i32gather_epi64((const long long*) &input[i], pick_l_indices, 8);
    // auto out_high = _mm256_i32gather_epi64((const long long*) &input[i], pick_h_indices, 8);

    auto pair_f4 = _mm256_loadu_si256((__m256i *)&input[i]);
    auto pair_s4 = _mm256_loadu_si256((__m256i *)&input[i + 4]);
    auto perm_f4 = _mm256_load_si256((__m256i*)&pre.permutation[mask]);
    auto perm_s4 = _mm256_load_si256((__m256i*)&pre.permutation[mask][8]);
    int count_f4 = __builtin_popcount((mask & 0b1111));
    int count_s4 = __builtin_popcount(mask) - count_f4;

    // print(pair_f4);
    // print(pair_s4);
    // print(perm_f4);
    // print(perm_s4);

    auto out_f4 = _mm256_permutevar8x32_epi32(pair_f4, perm_f4);
    auto out_s4 = _mm256_permutevar8x32_epi32(pair_s4, perm_s4);

    // print(out_f4);
    // print(out_s4);


    _mm256_storeu_si256((__m256i*)&output[k], out_f4);
    k += count_f4;
    // std::cout << k << std::endl;
    _mm256_storeu_si256((__m256i*)&output[k], out_s4);
    k += count_s4;

    // break;
  }

  // for (int i = 0; i < k; i++) {
  //   std::cout << output[i].first << " " << output[i].second << " ";
  // }
  // for (const auto item : input) {
  //   bool cond = (lower <= item.first) && (item.first <= upper);
  //   output[count] = item;
  //   count = count + cond;
  // }
  return k;
}
