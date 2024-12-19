
#include <array>
#include <cstdint>

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

using Blob = std::array<uint16_t, N>;

void init(Blob &blob);
uint16_t checksum(const Blob &blob);
