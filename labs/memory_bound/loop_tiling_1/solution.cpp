#include "solution.hpp"
#include <algorithm>

#define B 8

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
	int size = in.size();
	int i = 0;
	for (; i + B - 1 < size; i+=B) {
		int j = 0;
		for (; j + B - 1 < size; j+=B) {
			for (int ib = i; ib < i + B; ++ib) {
				for (int jb = j; jb < j + B; ++jb) {
					out[ib][jb] = in[jb][ib];
				}
			}
		}
		for (int ib = i; ib < i + B; ++ib) {
			for (int jb{j}; jb < size; jb++) {
				out[ib][jb] = in[jb][ib];
			}
		}
	}
	for (; i < size; i++) {
		for (int j = 0; j < size; j++) {
			out[i][j] = in[j][i];
		}
	}

	return out[0][size - 1];
}
