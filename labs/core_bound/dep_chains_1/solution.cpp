#include "solution.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <iostream>

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

// Task: lookup all the values from l2 in l1.
// For every found value, find the sum of its digits.
// Return the sum of all digits in every found number.
// Both lists have no duplicates and elements placed in *random* order.
// Do NOT sort any of the lists. Do NOT store elements in a hash_map/sets.

// Hint: Traversing a linked list is a long data dependency chain:
//       to get the node N+1 you need to retrieve the node N first.
//       Think how you can execute multiple dependency chains in parallel.
unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;
  List *head2 = l2;
  static constexpr int kBits = 256;
  static constexpr int kNumInt = kBits / sizeof(unsigned) / 8;
  while (l1) {
    unsigned temp[kNumInt]{0};
    for (int i = 0; i < kNumInt; i++) {
      if (l1) {
        temp[i] = l1->value;
        l1 = l1->next;
      }  
    }
    auto batch = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(temp));
    l2 = head2;
    int mask = 0;
    while (l2) {
      auto bc = _mm256_set1_epi32(l2->value);
      auto mask_256 = _mm256_cmpeq_epi32(batch, bc);
      mask |= _mm256_movemask_epi8(mask_256);
      l2 = l2->next;
    }

    for (int i = 0; i < kNumInt; i++) {
      if ((mask >> (i * 4)) & 1){
        retVal += getSumOfDigits(temp[i]);
      }
    }
  }
  // O(N^2) algorithm:
  //while (l1) {
  //  unsigned v = l1->value;
  //  l2 = head2;
  //  while (l2) {
  //    if (l2->value == v) {
  //      retVal += getSumOfDigits(v);
  //      break;
  //    }
  //    l2 = l2->next;
  //  }
  //  l1 = l1->next;
  //}

  return retVal;
}
