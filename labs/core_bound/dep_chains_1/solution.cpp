#include "solution.hpp"
#include <array>
#include <iostream>
#include <algorithm>

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
  constexpr int parallel_check_cnt = N;
  unsigned retVal = 0;

  List *head2 = l2;
  // O(N^2) algorithm:
  std::array<int, parallel_check_cnt> items;
  while (l1) {
    int items_cnt = 0;
    for (; items_cnt < parallel_check_cnt && l1; ++items_cnt) {
      items[items_cnt] = l1->value;
      l1 = l1->next;
    }
    l2 = head2;
    while (l2) {
      const auto items_iter =
        std::find(items.begin(), items.begin() + items_cnt, l2->value);
      if (items_iter != items.begin() + items_cnt) {
        retVal += getSumOfDigits(*items_iter);
        if (--items_cnt == 0) {
          break;
        }
        std::iter_swap(items_iter, items.begin() + items_cnt);
      }
      l2 = l2->next;
    }
  }

  return retVal;
}
