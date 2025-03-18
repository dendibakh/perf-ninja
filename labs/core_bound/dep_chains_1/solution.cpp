#include "solution.hpp"
#include <array>
#include <iostream>
#include <utility>

unsigned getSumOfDigits(unsigned n) {
  unsigned sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

std::pair<List*, List*> divideList(List* l) {
  List* head1 = l;
  List* head2 = l->next;
  List* f = l;
  List* s = l->next;
  while (s) {
    f->next = s->next;
    f = std::exchange(s, s->next);
  }
  return std::make_pair(head1, head2);
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
  auto [lh1, lh2] = divideList(l2);
  unsigned retVal = 0;

  List *head2 = l2;
  // O(N^2) algorithm:
  while (l1) {
    unsigned v = l1->value;
    l2 = head2;
    List* curr1 = lh1;
    List* curr2 = lh2;
    while (curr1 || curr2) {
      if (curr1) {
        if (curr1->value == v) {
          retVal += getSumOfDigits(v);
          break;
        }
        curr1 = curr1->next;
      }
      if (curr2) {
        if (curr2->value == v) {
          retVal += getSumOfDigits(v);
          break;
        }
        curr2 = curr2->next;
      }
    }
    l1 = l1->next;
  }

  return retVal;
}
