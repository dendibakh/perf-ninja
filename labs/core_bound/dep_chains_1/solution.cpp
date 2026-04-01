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

static unsigned getValue(List *&l1, List *l2, List *&q)
{
  if (l1 == nullptr)
  {
    q = nullptr;
    return 0;
  }
  auto val = l1->value;
  q = l2;
  l1 = l1->next;
  return val;
}

unsigned solution(List *l1, List *l2)
{
  unsigned retVal = 0;

  // O(N^2) algorithm:
  List *q0 = nullptr, *q1 = nullptr, *q2 = nullptr, *q3 = nullptr;
  auto v0 = getValue(l1, l2, q0);
  auto v1 = getValue(l1, l2, q1);
  auto v2 = getValue(l1, l2, q2);
  auto v3 = getValue(l1, l2, q3);
  while (q0 || q1 || q2 || q3)
  {
    if (q0)
    {
      if (q0->value == v0)
      {
        retVal += getSumOfDigits(v0);
        v0 = getValue(l1, l2, q0);
      }
      else
      {
        q0 = q0->next;
        if (q0 == nullptr)
          v0 = getValue(l1, l2, q0);
      }
    }

    if (q1)
    {
      if (q1->value == v1)
      {
        retVal += getSumOfDigits(v1);
        v1 = getValue(l1, l2, q1);
      }
      else
      {
        q1 = q1->next;
        if (q1 == nullptr)
          v1 = getValue(l1, l2, q1);
      }
    }

    if (q2)
    {
      if (q2->value == v2)
      {
        retVal += getSumOfDigits(v2);
        v2 = getValue(l1, l2, q2);
      }
      else
      {
        q2 = q2->next;
        if (q2 == nullptr)
          v2 = getValue(l1, l2, q2);
      }
    }

    if (q3)
    {
      if (q3->value == v3)
      {
        retVal += getSumOfDigits(v3);
        v3 = getValue(l1, l2, q3);
      }
      else
      {
        q3 = q3->next;
        if (q3 == nullptr)
          v3 = getValue(l1, l2, q3);
      }
    }
  }
  return retVal;
}
