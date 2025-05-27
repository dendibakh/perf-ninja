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

unsigned solution4blocks(List* l1, List* l2) {
    // 1) count total length of l2
    std::size_t total = 0;
    for (List* p = l2; p; p = p->next) {
        ++total;
    }
    if (total == 0) return 0;

    // 2) compute base size and remainder
    std::size_t q = total / 4;
    std::size_t r = total % 4;
    std::size_t len0 = q + (r > 3 ? 1 : 0);
    std::size_t len1 = q + (r > 2 ? 1 : 0);
    std::size_t len2 = q + (r > 1 ? 1 : 0);
    std::size_t len3 = q + (r > 0 ? 1 : 0);

    // 3) find the head pointer of each of the 4 blocks
    List* h0 = l2;
    List* p  = l2;
    for (std::size_t i = 0; i < len0 && p; ++i) p = p->next;
    List* h1 = p;
    for (std::size_t i = 0; i < len1 && p; ++i) p = p->next;
    List* h2 = p;
    for (std::size_t i = 0; i < len2 && p; ++i) p = p->next;
    List* h3 = p;

    unsigned retVal = 0;
    for (List* p1 = l1; p1; p1 = p1->next) {
        unsigned v = p1->value;

        List *c0 = h0, *c1 = h1, *c2 = h2, *c3 = h3;

        for (std::size_t i = 0; i < len0; ++i) {
            if (i >= len3) {
                break;
            }
            unsigned v0 = c0->value;
            unsigned v1 = c1->value;
            unsigned v2 = c2->value;
            unsigned v3 = c3->value;
            if (v0 == v) {
                retVal += getSumOfDigits(v);
                break;
            }

            if (v1 == v) {
                retVal += getSumOfDigits(v);
                break;
            }

            if (v2 == v) {
                retVal += getSumOfDigits(v);
                break;
            }

            if (v3 == v) {
                retVal += getSumOfDigits(v);
                break;
            }
            c0 = c0->next;
            c1 = c1->next;
            c2 = c2->next;
            c3 = c3->next;
        }
    }

    return retVal;
}

unsigned solution(List *l1, List *l2) {
  return solution4blocks(l1, l2);
}
