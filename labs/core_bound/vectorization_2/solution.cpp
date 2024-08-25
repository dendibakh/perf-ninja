#include "solution.hpp"
#include <limits>

uint16_t checksum(const Blob &blob) {
    constexpr int unroll = 16;
	uint16_t accs[unroll]{};
	std::size_t i = 0; 
	for (; i + unroll - 1 < blob.size(); i += unroll) {
		for (std::size_t j = 0; j < unroll; ++j) {
			accs[j] += blob[i + j];
			accs[j] += accs[j] < blob[i + j];
		}
	}
	for (; i < blob.size(); ++i) {
		accs[0] += blob[i];
		accs[0] += accs[0] < blob[i];
	}
	for (std::size_t j = 1; j < unroll; ++j) {
		accs[0] += accs[j];
		accs[0] += accs[0] < accs[j];
	}
	return accs[0];
}
