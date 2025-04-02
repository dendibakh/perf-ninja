#include "solution.hpp"
#include <iostream>

static constexpr std::array<std::size_t, 151> generateTable() {
  std::array<std::size_t, 151> table{};
  for (std::size_t v = 0; v < table.size(); ++v) {
    if      (v < 13)  table[v] = 0; //   13
    else if (v < 29)  table[v] = 1; //   16
    else if (v < 41)  table[v] = 2; //   12
    else if (v < 53)  table[v] = 3; //   12
    else if (v < 71)  table[v] = 4; //   18
    else if (v < 83)  table[v] = 5; //   12
    else if (v < 100) table[v] = 6; //   17
    else table[v] = 7;
  }
  return table;
 }

static constexpr std::size_t mapToBucket(std::size_t v) {
  return generateTable()[v];
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  constexpr auto table = generateTable();
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[table[v]]++;
  }
  return retBuckets;
}
