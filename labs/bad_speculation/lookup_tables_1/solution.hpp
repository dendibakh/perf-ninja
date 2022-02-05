#include <array>
#include <vector>

constexpr size_t NUM_BUCKETS = 7;
constexpr size_t NUM_VALUES = 1024 * 1024;

void init(std::vector<int> &values);
std::array<size_t, NUM_BUCKETS> histogram(const std::vector<int> &values);
