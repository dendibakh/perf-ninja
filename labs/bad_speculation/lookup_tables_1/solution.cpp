#include "solution.hpp"

#include <algorithm>
#include <cstdint>

#ifndef SOLUTION
  #define SOLUTION 1
#endif

#if SOLUTION == 0
// Baseline.

static std::size_t mapToBucket(std::size_t v) {    // diff
  if (v >= 0 && v < 13)        return 0; // 13
  else if (v >= 13 && v < 29)  return 1; // 16
  else if (v >= 29 && v < 41)  return 2; // 12
  else if (v >= 41 && v < 53)  return 3; // 12
  else if (v >= 53 && v < 71)  return 4; // 18
  else if (v >= 71 && v < 83)  return 5; // 12
  else if (v >= 83 && v < 100) return 6; // 17
  return -1;  // let it crash
}

#elif SOLUTION == 1
// My solution.

uint8_t mapToBucketTable[100];

/**
 * Set all values in the given range of the mapToBucketTable to the given value.
 *
 * @param begin Index of the first element in the range.
 * @param end Index one-past the lat element in the range.
 * @param value The value to assign to each element in the range.
 */
void initTableRange(std::size_t begin, std::size_t end, uint8_t value)
{
  while (begin != end)
  {
    mapToBucketTable[begin] = value;
    ++begin;
  }
}


void initializeTable()
{
  initTableRange(0, 13, 0);
  initTableRange(13, 29, 1);
  initTableRange(29, 41, 2);
  initTableRange(41, 53, 3);
  initTableRange(53, 71, 4);
  initTableRange(71, 83, 5);
  initTableRange(83, 100, 6);
}

struct InitializeTable
{
  InitializeTable()
  {
    initializeTable();
  }
};

static InitializeTable g_initializeTable;

static uint8_t mapToBucket(std::size_t v)
{
  return mapToBucketTable[v];
}


#elif SOLUTION == 2
// Facit.

int buckets[100] = {
// 1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};

static std::size_t mapToBucket(std::size_t v) {
  if (v < (sizeof (buckets) / sizeof (int)))
    return buckets[v];
  return -1; // let it crash
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif

std::array<std::size_t, NUM_BUCKETS> histogram(std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
