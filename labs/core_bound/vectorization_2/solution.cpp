#include "solution.hpp"

uint16_t checksum(const Blob &blob) {
	uint32_t acc{};
	for (auto value : blob) acc += value;

	auto high = acc>>16;
	auto low = acc&0xffff;
	acc = low + high;

	high = acc>>16;
	low = acc&0xffff;
	acc = low + high;

	return static_cast<uint16_t>(acc);
}
