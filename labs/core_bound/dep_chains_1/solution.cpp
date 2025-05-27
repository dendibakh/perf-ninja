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
  List *head2 = l2;
  List *mid_head2 = l2;
  unsigned retVal = 0;
  int len2 = 0;

  while (l2) {
    len2++;
    l2 = l2->next;
  }
  len2 = len2/2;
  while (len2--) {
    mid_head2 = mid_head2->next;
  }
  List *ml2 = mid_head2;
  while (l1) {
    unsigned v = l1->value;
    l2 = head2;
    ml2 = mid_head2;
    while (ml2 != nullptr) {
      unsigned v2 = l2->value;
      unsigned mv2 = ml2->value;
      l2 = l2->next;
      ml2 = ml2->next;
      if (v2 == v || mv2 == v) {
        retVal += getSumOfDigits(v);
        break;
      }
    }
    l1 = l1->next;
  }

  return retVal;
}