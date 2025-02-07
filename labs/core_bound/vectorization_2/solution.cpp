#include "solution.hpp"

#define N 16

uint16_t checksum(const Blob &blob) {
	uint16_t acc[N]{};
	int i{};
	for (; i + N <= blob.size(); i += N) {
		for (int j{}; j < N; ++j) {
			acc[j] += blob[i+j];
			acc[j] += acc[j] < blob[i+j];
		}
	}
	for (; i < blob.size(); ++i) {
		acc[0] += blob[i];
		acc[0] += acc[0] < blob[i];
	}
	for (int j{1}; j < N; ++j) {
		acc[0] += acc[j];
		acc[0] += acc[0] < acc[j];
	}
	return acc[0];
}
