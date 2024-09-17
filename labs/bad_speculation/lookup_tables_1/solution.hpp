#include <array>
#include <vector>

constexpr std::size_t NUM_BUCKETS = 8;
constexpr std::size_t DEFAULT_BUCKET = NUM_BUCKETS - 1;
constexpr std::size_t NUM_VALUES = 1024 * 1024;

void init(std::vector<int> &values);
std::array<std::size_t, NUM_BUCKETS> histogram(const std::vector<int> &values);
