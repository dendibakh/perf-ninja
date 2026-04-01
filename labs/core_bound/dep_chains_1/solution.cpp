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

unsigned solution(List *l1, List *l2)
{
  unsigned retVal = 0;

  // O(N^2) algorithm:
  constexpr int chains = 4;
  std::array<List *, chains> q{};
  std::array<unsigned, chains> v{};
  for (auto i = 0; i < chains; ++i)
  {
    if (l1)
    {
      v[i] = l1->value;
      l1 = l1->next;
      q[i] = l2;
    }
  }
  bool any_active = true;
  while (any_active)
  {
    any_active = false;
    for (auto i = 0; i < chains; ++i)
    {
      if (q[i])
      {
        any_active = true;
        if (q[i]->value == v[i]) [[unlikely]]
        {
          retVal += getSumOfDigits(v[i]);
          q[i] = nullptr;
        }
        else
        {
          q[i] = q[i]->next;
        }
      }

      if (!q[i] && l1) [[unlikely]]
      {
        v[i] = l1->value;
        l1 = l1->next;
        q[i] = l2;
      }
    }
  }
  return retVal;
}
