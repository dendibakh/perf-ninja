#include "solution.hpp"
#include <algorithm>

constexpr int BLKSZ = 128;

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i += BLKSZ) {
    for (int j = 0; j < size; j += BLKSZ) {
      for (int i2 = i; i2 < std::min(i + BLKSZ, size); i2++) {
        for (int j2 = j; j2 < std::min(j + BLKSZ, size); j2++) {
          out[i2][j2] = in[j2][i2];
        }
      }
    }
  }
  return out[0][size - 1];
}
