#include "solution.hpp"
#include <array>
#include <vector>
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

  std::vector<unsigned> l1_values;
  while (l1) {
    l1_values.push_back(l1->value);
    l1 = l1->next;
  }

  // O(N^2) algorithm:
  while (l2) {
    for (unsigned v : l1_values) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        break;
      }
    }
    l2 = l2->next;
  }

  return retVal;
}
