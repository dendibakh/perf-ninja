#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
    uint32_t sum = 0;
    for (auto value: blob) {
        sum += value;
    }

    uint16_t high = sum >> 16;
    uint16_t low = sum & ((1 << 16) - 1);
    low += high;
    low += low < high;

    return low;
}
