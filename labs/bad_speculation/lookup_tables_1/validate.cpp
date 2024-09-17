
#include "solution.hpp"
#include <iostream>

static size_t mapToBucket(size_t v) {
                              //   size of a bucket
  if      (v < 13)  return 0; //   13
  else if (v < 29)  return 1; //   16
  else if (v < 41)  return 2; //   12
  else if (v < 53)  return 3; //   12
  else if (v < 71)  return 4; //   18
  else if (v < 83)  return 5; //   12
  else if (v < 100) return 6; //   17
  return DEFAULT_BUCKET;
}

std::array<size_t, NUM_BUCKETS>
original_histogram(const std::vector<int> &values) {
  std::array<size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}

int main() {
  std::vector<int> values;
  values.reserve(NUM_VALUES);
  init(values);

  auto original_result = original_histogram(values);
  auto result = histogram(values);

  if (original_result != result) {
    std::cerr << "Validation Failed."
              << "\n";
    std::cerr << "Original result: ";
    for (auto v : original_result)
      std::cerr << v << " ";
    std::cerr << "\n";
    std::cerr << "Modified version returned: ";
    for (auto v : result)
      std::cerr << v << " ";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
