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
#else

#include <array>

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

using Val = unsigned int;

static constexpr std::array<Val, 100> gen_lookup()
{
  std::array<Val, 100> arr{};
  for (size_t i = 0; i < 100; i++)
  {
    arr[i] = (Val)mapToBucket_ori(i);
  }

  return arr;
}

// static constexpr std::array<Val, 100> lookup = gen_lookup();

constexpr uint8_t lookup[] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U};

static std::size_t mapToBucket(std::size_t v)
{
  if (v < sizeof(lookup) / sizeof(lookup[0]))
    return lookup[v];
  return DEFAULT_BUCKET;
}
#endif

std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values)
{
  std::array<std::size_t, NUM_BUCKETS> retBuckets{0};
  for (auto v : values)
  {
    retBuckets[mapToBucket(v)]++;
  }
  return retBuckets;
}
