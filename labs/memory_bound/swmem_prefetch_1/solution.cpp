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
  PREFETCH(hash_map);
  PREFETCH(lookups.data());
  constexpr std::size_t lookahead = 16;
  const std::size_t len = lookups.size();
  const std::size_t len_short = len - lookahead;
  std::size_t i = 0;
  for (; i < len_short; ++i) {
    const int val = lookups[i];
    hash_map->prefetch(lookups[i + lookahead]);
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
  for (; i < len; ++i) {
    const int val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
#elif defined(SOLUTION2)
  PREFETCH(hash_map);
  const auto begin = lookups.cbegin();
  const auto final_iter = --lookups.cend();
  for (auto iter = begin; iter != final_iter;)
  {
    const int val = *iter;
    const auto next = std::next(iter);
    hash_map->prefetch(*next);
    if (hash_map->find(val))
      result += getSumOfDigits(val);
    iter = next;
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
