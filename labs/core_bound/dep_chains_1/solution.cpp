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

  List *head2 = l2;
  // 2 at the same time.
  while (l1 && l1->next && l1->next->next && l1->next->next->next) {
    unsigned v = l1->value;
    unsigned w = l1->next->value;
    unsigned x = l1->next->next->value;
    unsigned y = l1->next->next->next->value;

    int found = 0;
    l2 = head2;
    while (l2) {
      if (l2->value == v) {
        retVal += getSumOfDigits(v);
        found += 1;
      }
      if (l2->value == w) {
        retVal += getSumOfDigits(w);
        found += 1;
      }
      if (l2->value == x) {
        retVal += getSumOfDigits(x);
        found += 1;
      }
      if (l2->value == y) {
        retVal += getSumOfDigits(y);
        found += 1;
      }
      if (found == 4) {
        break;
      }
      l2 = l2->next;
    }
    // Handle odd number
    if (!l1->next || !l1->next->next || !l1->next->next->next) {
      break;
    }
    l1 = l1->next->next->next->next;
  }
  // If uneven -> Process last entry.
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
