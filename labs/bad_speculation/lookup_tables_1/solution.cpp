#include "solution.hpp"

#define SOLUTION
#ifndef SOLUTION
static std::size_t mapToBucket(std::size_t v)
{
  //   size of a bucket
  if (v < 13)
    return 0; //   13
  else if (v < 29)
    return 1; //   16
  else if (v < 41)
    return 2; //   12
  else if (v < 53)
    return 3; //   12
  else if (v < 71)
    return 4; //   18
  else if (v < 83)
    return 5; //   12
  else if (v < 100)
    return 6; //   17
  return DEFAULT_BUCKET;
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values)
{
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values)
  {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}

#else

#include <array>
#include <cstdint>

static constexpr std::size_t mapToBucket_ori(const std::size_t v)
{
  //   size of a bucket
  if (v < 13)
    return 0; //   13
  else if (v < 29)
    return 1; //   16
  else if (v < 41)
    return 2; //   12
  else if (v < 53)
    return 3; //   12
  else if (v < 71)
    return 4; //   18
  else if (v < 83)
    return 5; //   12
  else if (v < 100)
    return 6; //   17
  return DEFAULT_BUCKET;
}

constexpr size_t N = 151;
using Val = std::uint8_t;

static constexpr std::array<Val, N> gen_lookup()
{
  std::array<Val, N> arr{};
  for (size_t i = 0; i < N; i++)
  {
    arr[i] = (Val)mapToBucket_ori(i);
  }

  return arr;
}

static constexpr std::array<Val, N> lookup = gen_lookup();

static std::size_t mapToBucket(std::size_t v)
{
  if (v < lookup.size())
    return lookup[v];
  return DEFAULT_BUCKET;
}

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values)
{
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (size_t i = 0; i < values.size(); i++)
  {
    retBuckets[mapToBucket(values[i])]++;
  }
  return retBuckets;
}

#endif
