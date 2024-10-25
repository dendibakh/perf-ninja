#include "solution.hpp"
#include <array>
#include <iostream>
#include <unordered_set>

// #define SOLUTION

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

unsigned solution_improved(List *l1, List *l2) {
  std::unordered_set<unsigned> cache;
  while (l2) {
    cache.insert(l2->value);
    l2 = l2->next;
  }
  unsigned retVal = 0;
  while (l1) {
    if (cache.count(l1->value)) {
      retVal += getSumOfDigits(l1->value);
    }
    l1 = l1->next;
  }
  return retVal;

  // unsigned retVal = 0;

  // List *head2{l2};
  // constexpr int capacity{4048};
  // std::array<unsigned,capacity> cached{};
  // int cache_len{0};

  // while (l1) {
  //   cache_len = 0;
  //   for (int i = 0; l1 && i < capacity; i++) {
  //     cached[cache_len++] = l1->value;
  //     l1 = l1->next;
  //   }

  //   l2 = head2;
  //   while (l2) {
  //     for (int i = 0; i < cache_len; i++) {
  //       if (cached[i] == l2->value) {
  //         retVal += getSumOfDigits(cached[i]);
  //         std::swap(cached[i--], cached[(cache_len--)-1]);
  //       }
  //     }
  //     if (!cache_len) break;
  //     l2 = l2->next;
  //   }

  // }
  // return retVal;
}

unsigned solution_baseline(List *l1, List *l2) {
  unsigned retVal = 0;

  List *head2 = l2;
  // O(N^2) algorithm:
  while (l1) {
    unsigned v = l1->value;
    l2 = head2;
    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }

  return retVal;
}

unsigned solution(List *l1, List *l2) {
#ifdef SOLUTION
  return solution_improved(l1, l2);
#else
  return solution_baseline(l1, l2);
#endif
}
