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
  unsigned sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;

  List *head2 = l2;
  unsigned lane = 0;

  while (l1) {
    unsigned v = l1->value;
    l2 = head2;

    while (l2) {
      if (l2->value == v) {
        unsigned s = getSumOfDigits(v);
        switch (lane & 3) {
          case 0: sum0 += s; break;
          case 1: sum1 += s; break;
          case 2: sum2 += s; break;
          case 3: sum3 += s; break;
        }
        ++lane;
        break;
      }
      l2 = l2->next;
    }

    l1 = l1->next;
  }

  return sum0 + sum1 + sum2 + sum3;
}