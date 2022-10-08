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
  unsigned retVal = 0;

  List * const head2 = l2;

  const int buffSize = N;
  static unsigned buffer[buffSize];

  // Complexity: N + N/S * (S*logS + N*logS), S := buffer size
  while (l1) {
    int i = 0;
    for (; l1 && i < buffSize; ++i) {
        buffer[i] = l1->value;
        l1 = l1->next;
    }

    auto begin = buffer;
    auto end = buffer + i;

    std::sort(begin, end);

    l2 = head2;
    while (l2) {
      const unsigned l2value = l2->value;

      if (bool found = std::binary_search(begin, end, l2value); found) {
        retVal += getSumOfDigits(l2value);
      }

      l2 = l2->next;
    }
  }

  return retVal;
}
