#include "solution.hpp"
#define SOLUTION
// #undef SOLUTION

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
  // if the one loop prefetch window is too small to give any benefit
  // (the data hasn't been fetched when it's needed, computations can't hide latency)
  // we can just request data for lookAhead iterations forward.
  // several iterations are more likely to hide latency
  constexpr int lookAhead = 16;

  for (int idx = 0; idx < lookups.size(); ++idx) {
    int val = lookups[idx];
    hash_map->preload(lookups[idx + lookAhead]);
#else
  for (int val : lookups) {
#endif
  
    if (hash_map->find(val)) {
      result += getSumOfDigits(val);
    }
  }

  return result;
}
