#pragma once
#include <array>
#include <vector>

constexpr std::size_t NUM_BUCKETS = 8;
constexpr std::size_t DEFAULT_BUCKET = NUM_BUCKETS - 1;
constexpr std::size_t NUM_VALUES = 1024 * 1024;

#define SOLUTION

#ifdef SOLUTION

void init(std::vector<int> &values, std::vector<std::size_t> & table);
std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values, std::vector<std::size_t> const & table);

#else
void init(std::vector<int> &values);
std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values);
#endif
