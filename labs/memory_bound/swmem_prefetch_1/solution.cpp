#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

int solution(const hash_map_t* hash_map, const std::vector<int>& lookups) {
  int result = 0;

  constexpr size_t kStride = 16;
  for (size_t i = 0; i < lookups.size() - kStride; i++) {
    hash_map->prefetch(lookups[i + kStride]);
    if (hash_map->find(lookups[i])) {
      result += getSumOfDigits(lookups[i]);
    }
  }
  for (size_t i = lookups.size() - kStride; i < lookups.size(); i++) {
    if (hash_map->find(lookups[i])) {
      result += getSumOfDigits(lookups[i]);
    }
  }

  return result;
}
