
#include "solution.hpp"
#include <iostream>
#include <memory>

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

static int original_solution(const hash_map_t *hash_map,
                             const std::vector<int> &lookups) {
  int result = 0;

  for (int val : lookups) {
    if (hash_map->find(val))
      result += getSumOfDigits(val);
  }

  return result;
}

int main() {
  // Init benchmark data
  auto hash_map = std::make_unique<hash_map_t>(HASH_MAP_SIZE);
  std::vector<int> lookups;
  lookups.reserve(NUMBER_OF_LOOKUPS);
  init(hash_map.get(), lookups);

  auto original_result = original_solution(hash_map.get(), lookups);
  auto result = solution(hash_map.get(), lookups);

  if (original_result != result) {
    std::cerr << "Validation Failed. Original result = " << original_result
              << "; Modified version returned = " << result << "\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
