#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
    std::array<uint32_t, 8> seg_sum{};
    seg_sum.fill(0);
    for (std::size_t i = 0; i < N; i += 8) {
        for (std::size_t j = 0; j < 8; j++) {
            seg_sum[j] += blob[i + j];
        }
    }
    uint32_t sum = 0;
    for (std::size_t i = 0; i < 8; i++) {
        sum += seg_sum[i];
    }
    uint16_t high = sum >> 16;
    uint16_t low = sum & ((1 << 16) - 1);
    low += high;
    low += low < high;

    return low;
}
