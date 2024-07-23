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

    List *head1 = l1;
    List *head2 = l2;
    // O(N^2) algorithm:

    const int unroll = 32;

    while (l1) {
        int vals[unroll];
        int num_seen = 0;

        for (int i = 0; i < unroll && l1; ++i, l1 = l1->next)
            vals[num_seen++] = l1->value;


        for (l2 = head2; l2 && num_seen > 0; l2 = l2->next) {
            for (int i = 0; i < num_seen;) {
                if (vals[i] == l2->value) {
                    retVal += getSumOfDigits(vals[i]);
                    std::swap(vals[i], vals[--num_seen]);
                } else {
                    ++i;
                }
            }
        }
    }

    return retVal;
}
