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
  constexpr unsigned N = 1;
  unsigned retVal = 0;
  std::array<unsigned, N> res;
  for (unsigned k{}; k < N; ++k)
    res[k] = 0;


  while (l2) {
    std::array<unsigned, N> values;
    for (unsigned k{}; k < N; ++k) {
      if (l2) {
        values[k] = l2->value;
        l2 = l2->next;
      } else {
        values[k] = 0;
      }
    }

    List *head1 = l1;
    unsigned rem = N;
    while (head1) {
      unsigned v = head1->value;
      for (unsigned k{}; k < N; ++k) {
        if (v == values[k]) {
          res[k] += getSumOfDigits(v);
          values[k] = 0; // TODO swap with last element and compare N-1
          //rem -= 1;
          break;
        }
      }
      //if (!rem)
      //  break;
      head1 = head1->next;
    }
  }

  for (unsigned k{}; k < N; ++k)
    retVal += res[k];

  return retVal;
}
