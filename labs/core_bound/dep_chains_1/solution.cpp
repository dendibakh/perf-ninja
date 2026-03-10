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
unsigned solution(List* l1, List* l2) {
  constexpr int M = 16;
  unsigned retVal = 0;

  List* head2 = l2;

  while (l1) {
    std::array<unsigned, M> vals{};
    std::array<bool, M> active{};
    int count = 0;

    // Load up to 8 values from l1
    for (; count < M && l1; ++count) {
      vals[count] = l1->value;
      active[count] = true;
      l1 = l1->next;
    }

    // Scan l2 once for the whole batch
    l2 = head2;
    int found = 0;

    while (l2 && found < count) {
      unsigned v2 = l2->value;

      for (int j = 0; j < count; ++j) {
        if (active[j] && v2 == vals[j]) {
          retVal += getSumOfDigits(vals[j]);
          active[j] = false;  // count each l1 element at most once
          ++found;
        }
      }

      l2 = l2->next;
    }
  }

  return retVal;
}