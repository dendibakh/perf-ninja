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

  std::vector<int> found;
  found.reserve(lookups.size());

  for (size_t i = 0; i < lookups.size(); i++) {
    if (hash_map->find(lookups[i])) {
      found.push_back(lookups[i]);
    }
  }

  for (int val : found) {
    result += getSumOfDigits(val);

  return result;
}
