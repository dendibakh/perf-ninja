#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;

#ifdef SOLUTION
  const auto begin = lookups.cbegin();
  const auto final_iter = --lookups.cend();
  for (auto iter = begin; iter != final_iter; ++iter)
  {
    const auto next = std::next(iter);
    hash_map->prefetch(*next);
    const int val = *iter;
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
  const int val = *final_iter;
  if (hash_map->find(val))
      result += getSumOfDigits(val);
#else
  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
#endif

  return result;
}
