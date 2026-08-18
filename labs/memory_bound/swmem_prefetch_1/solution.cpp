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
  static constexpr int WINDOW = 16;
  const int sz = lookups.size();
  for (int i = 0; i < sz - WINDOW; ++i) {
    auto val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
    hash_map->pref(lookups[i + WINDOW]);
  }
  for (int i = sz - WINDOW; i < sz; ++i) {
    auto val = lookups[i];
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}
