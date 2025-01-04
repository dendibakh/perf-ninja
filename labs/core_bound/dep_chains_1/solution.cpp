#include "solution.hpp"
#include <array>
#include <iostream>

unsigned getSumOfDigits(unsigned n)
{
  unsigned sum = 0;
  while (n != 0)
  {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

#define SOLUTION
#ifdef SOLUTION

unsigned solution(List *l1, List *l2)
{

  List *const head1 = l1;

  size_t l1_len = 0;
  while (l1)
  {
    ++l1_len;
    l1 = l1->next;
  }
  l1 = head1;

  constexpr size_t cache_len = 64 / sizeof(unsigned);
  std::array<unsigned, cache_len> cache{};

  const size_t cache_blocks = l1_len / cache_len;

  unsigned retVal = 0;

  List *const head2 = l2;
  // O(N^2) algorithm:
  for (size_t k = 0; k < cache_blocks; k++)
  {
    for (size_t i = 0; i < cache_len; i++)
    {
      cache[i] = l1->value;
      l1 = l1->next;
    }

    size_t cache_occupancy = cache_len;

    l2 = head2;
    while (l2)
    {
      for (size_t i = 0; i < cache_occupancy; i++)
      {
        const unsigned v = cache[i];
        if (l2->value == v)
        {
          retVal += getSumOfDigits(v);

          std::swap(cache[i], cache[cache_occupancy - 1]);
          --cache_occupancy;
          break;
        }
      }

      if (cache_occupancy == 0)
      {
        break;
      }

      l2 = l2->next;
    }
  }

  while (l1)
  {
    unsigned v = l1->value;
    l2 = head2;
    while (l2)
    {
      if (l2->value == v)
      {
        retVal += getSumOfDigits(v);
        break;
      }
      l2 = l2->next;
    }
    l1 = l1->next;
  }

  return retVal;
}

#else

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

  List *head2 = l2;
  // O(N^2) algorithm:
  while (l1)
  {
    unsigned v = l1->value;
    l2 = head2;
    while (l2)
    {
      if (l2->value == v)
      {
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