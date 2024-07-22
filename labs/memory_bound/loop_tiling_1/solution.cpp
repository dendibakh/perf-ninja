#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
    int size = in.size();
    const int k = 16;
    for (int lo_i = 0, hi_i = k; lo_i < size; lo_i = hi_i, hi_i = std::min(size, hi_i + k)) {
        for (int lo_j = 0, hi_j = k; lo_j < size; lo_j = hi_j, hi_j = std::min(size, hi_j + k)) {
            for (int i = lo_i; i < hi_i; i++) {
                for (int j = lo_j; j < hi_j; j++) {
                    out[i][j] = in[j][i];
                }
            }
        }
    }

    return out[0][size - 1];
}
