#include "solution.hpp"

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

static size_t mapToBucket2(size_t v) {
	size_t index{ 0 };

	index += v >= 13;
	index += v >= 29;
	index += v >= 41;
	index += v >= 53;
	index += v >= 71;
	index += v >= 83;
	
	return index;
}

constexpr size_t MAX_VALUE = 100;
using lookupArray_t = std::array<size_t, MAX_VALUE>;

constexpr lookupArray_t fillLookupArray() {
	lookupArray_t lookupArray{};
	constexpr std::array steps = { 13,29,41,53,71,83,100 };

	for (size_t i = 1; i < steps.size(); ++i) {
		const int from = steps[i - 1];
		const int to = steps[i];

		for (size_t j = from; j < to; ++j) {
			lookupArray[j] = i;
		}
	}

	return lookupArray;
}

constexpr lookupArray_t lookupArray = fillLookupArray();

static size_t mapToBucket3(size_t v) {
	return lookupArray[v];
}

std::array<size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<size_t, NUM_BUCKETS> retBuckets{0};

  for (int v : values) {
    retBuckets[mapToBucket3(v)]++;
  }

  return retBuckets;
}
