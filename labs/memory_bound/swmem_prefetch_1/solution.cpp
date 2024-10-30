#include "solution.hpp"
#include <cstddef>

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
  constexpr size_t prefetch_elements = 5;
    for (int j = 0; j < std::min(prefetch_elements, lookups.size()); j++) {
      __builtin_prefetch(hash_map->getPtr(lookups[j]));
    }
  for (size_t i = 0; i < lookups.size(); ++i) {
    auto val = lookups[i];
    if (i + prefetch_elements < lookups.size()) {
      __builtin_prefetch(hash_map->getPtr(lookups[i + prefetch_elements]));
    }
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  // for (int val : lookups) {
  //   if (hash_map->find(val))
  //     result += getSumOfDigits(val);
  // }
#else
  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }
#endif

  return result;
}
