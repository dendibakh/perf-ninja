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

// To improve the dependency chain execution, instead of going through the list and evaluate each node
// We will overlap the dependency chains by going through the list in chunks.
// This function returns the sum of the digits in the values that occur in both the first and second lists.
// The lists are assumed to be sorted in ascending order.
// The function traverses the first list and stores the first M elements in an array. It then traverses the second
// list and compares each element in the second list to each element in the array. If the elements match, the
// function calculates the sum of the digits of the matching element and adds it to the return value.

template <int M> unsigned solution(List *l1, List *l2) {
  unsigned retVal = 0;
  List *head2 = l2;
  List *head1 = l1;

  int length1 = 0;
  while (l1) {
    length1++;
    l1 = l1->next;
  }

  l1 = head1;

  // Simultaneously lookup M elements in l1 and l2.
  for (int i = 0; i < length1 / M; i++) {
    std::array<unsigned, M> vals;
    // remember M values from l1 and l2
    for (int j = 0; j < M; j++) {
      vals[j] = l1->value;
      l1 = l1->next;
    }
    // traverse l2 and lookup M elements from vals at the same time
    l2 = head2;
    while (l2) {
      for (int j = 0; j < M; j++) {
        if (l2->value == vals[j]) {
          retVal += getSumOfDigits(l2->value);
        }
      }
      l2 = l2->next;
    }
  }

  return retVal;
}

unsigned solution(List *l1, List *l2) {
  return ::solution<4>(l1, l2);
}
