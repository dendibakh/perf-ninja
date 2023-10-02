#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}


#ifndef SOLUTION
#  define SOLUTION 1
#endif

#if SOLUTION == 0
// Baseline.

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}


#elif SOLUTION == 1
// My solution.


int solution(const hash_map_t *hash_map, const std::vector<int> &lookups)
{
  int result = 0;

  if (lookups.size() == 0)
  {
    return result;
  }

  int current_val = lookups[0];
  hash_map->prefetch(current_val);
  for (std::size_t i = 1; i < lookups.size(); ++i)
  {
    int next_val = lookups[i];
    hash_map->prefetch(next_val);
    if (hash_map->find(current_val))
      result += getSumOfDigits(current_val);
    current_val = next_val;
  }
  if (hash_map->find(current_val))
    result += getSumOfDigits(current_val);

  return result;
}


#elif SOLUTION == 2
// Facit.

// TODO Still the baseline implementation.

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif

