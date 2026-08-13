#include "solution.hpp"
#include <array>
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

  std::array<unsigned, N> a1, a2;
  size_t idx1 = 0, idx2 = 0;
  while (l1) {
    a1[idx1++] = l1->value;
    l1 = l1->next;
  }
  while (l2) {
    a2[idx2++] = l2->value;
    l2 = l2->next;
  }
  // std::cout << "XX " << N << " " << idx << std::endl;
  for (size_t i = 0; i < idx1; ++i) {
    for (size_t j = 0; j < idx2; ++j) {
      if (a1[i] == a2[j]) {
        retVal += getSumOfDigits(a1[i]);
        break;
      }
    }
  }

  return retVal;
}
