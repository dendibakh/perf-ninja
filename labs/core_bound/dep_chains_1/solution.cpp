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

constexpr unsigned P = 10000;

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
  std::array<unsigned, P> vals;

  List *head2 = l2;
  // O(N^2) algorithm:
  while (l1) {
    size_t i;
    for (i = 0; i < P; i++) {
      vals[i] = l1->value;
      l1 = l1->next;
      if (!l1) {
        i++;
        break;
      }
    }
    l2 = head2;
    while (l2) {
      for (size_t j = 0; j < i; j++) {
        if (vals[j] == l2->value) retVal += getSumOfDigits(vals[i]);
        break;
      }
      l2 = l2->next;
    }
    if (!l1) break;
    l1 = l1->next;
  }

  return retVal;
}
