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

  for (auto it = lookups.begin(); it != lookups.end(); ) {
    int val = *it;
    bool found = hash_map->find(val);
    ++it;
    if (it != lookups.end()) {
      hash_map->fetch(*it);
    }
    if (found)
      result += getSumOfDigits(val);
  }
  return result;
}
