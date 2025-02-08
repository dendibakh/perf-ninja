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
#define L1N 64
unsigned solution(List *l1, List *l2) {
	unsigned retVal = 0;
	List *head2 = l2;
	while (l1) {
		unsigned v1[L1N]{};
		for (int i{}; i < L1N and l1; ++i) {
			v1[i] = l1->value;
			l1 = l1->next;
		}
		l2 = head2;
		int n_found{};
		while (l2 and n_found < L1N) {
			for (int i{}; i < L1N; ++i) {
				if (l2->value == v1[i]) {
					retVal += getSumOfDigits(l2->value);
					++n_found;
					break;
				}
			}
			l2 = l2->next;
		}
	}

	return retVal;
}
