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
  static constexpr int UNUSED = std::numeric_limits<int>::max();
  int result = 0;
  std::size_t nBuckets = hash_map->get_nBuckets();
  std::vector<int> v = hash_map->get_vector();
  int look_ahead = 16;
  auto size = lookups.size();
  int steps = 0;

  for (int i=0; i<size-look_ahead; ++i) {

    if (v[lookups[i]%nBuckets] != UNUSED)
      result += getSumOfDigits(lookups[i]);

    __builtin_prefetch(&v[lookups[i + look_ahead]%nBuckets], 0, 1);
  }

  for(int i=size-look_ahead; i<size; ++i){
    if (v[lookups[i]%nBuckets] != UNUSED)
      result += getSumOfDigits(lookups[i]);
  }

  return result;
}