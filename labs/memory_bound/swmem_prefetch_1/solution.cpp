#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

static constexpr auto prefetch_step = 16;
int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  const auto size = lookups.size(); 

  if (size <= prefetch_step) {
    for (std::size_t i = 0; i < size; i++) {
      if (const int val = lookups[i]; hash_map->find(val)) {
        result += getSumOfDigits(val);
      }
    }
    return result;
  }

  for (auto i = 0; i + prefetch_step < size; i++) {
    if (const int val = lookups[i]; hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
    hash_map->prefetch_find(lookups[i + prefetch_step]);
  }

  for (auto i = size - prefetch_step; i < size; i++) {
    if (const int val = lookups[i]; hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
