#include "solution.hpp"

// This table is used to look up the bucket number for a given
// value. It is used by the lookup table to quickly find the
// bucket where a given key is stored.
//
// The BUCKET_LOOKUP array is used to determine the initial
// bucket to search.  The range value is used as an index into
// the BUCKET_LOOKUP array. The value at that index is the
// bucket number to search.
//
// The BUCKET_LOOKUP array is ordered by hash value.  The
// values in the array are the bucket numbers.  The index value
// will be between 0 and 99.  The BUCKET_LOOKUP array is
// indexed by hash value.
// This will remove most of the branches from the code. And tremendously help with performance

static constexpr std::size_t MAX_RANGE = 100;
static constexpr std::size_t BUCKET_LOOKUP[MAX_RANGE] = {
  // Map limits to bucket indices
//  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0 - 12
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 13 - 28
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      // 29 - 40
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      // 41 - 52
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   // 53 - 70
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,           // 71 - 82
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6    // 83 - 99
};

static std::size_t mapToBucket(std::size_t v) {
  // Calculate the bucket index for the value v
  // but make sure that v is in the range [0, 99]
  // and return -1 if it is not.
  if(v < (sizeof(BUCKET_LOOKUP)/sizeof(BUCKET_LOOKUP[0])))
      return BUCKET_LOOKUP[v];
  else
      return -1;
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values) {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
