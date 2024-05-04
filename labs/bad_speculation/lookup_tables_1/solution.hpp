#include <array>
#include <vector>

constexpr std::size_t NUM_BUCKETS = 7;
constexpr std::size_t NUM_VALUES = 1024 * 1024 * 100;

void init(std::vector<int> &values);
std::array<std::size_t, NUM_BUCKETS> histogram(std::vector<int> &values);
