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
#define SOLUTION
#ifdef SOLUTION
unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;
  constexpr int N = 8;
  std::array<int, N> val;
  int to_find = 0;

  List *head2 = l2;
  // O(N^2) algorithm:
  int found=0;
  while (l1) {
    to_find = 0;
    found = 0;
    while(l1 && to_find < N){
      val[to_find]=l1->value;
      ++to_find;
      l1 = l1->next;
    }

    l2 = head2;
    while (l2) {
      for(int i=0; i<to_find; ++i){
        if(l2->value == val[i]){
          retVal += getSumOfDigits(val[i]);
          found++;
          if(found == to_find){ break; }
        }
      }
      if(found == to_find){ break; }

      l2 = l2->next;
    }
  }

  return retVal;
}
#else //baseline
unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;

  List *head2 = l2;
  // O(N^2) algorithm:
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
#endif
