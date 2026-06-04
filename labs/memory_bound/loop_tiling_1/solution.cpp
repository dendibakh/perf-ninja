#include "solution.hpp"
#include <algorithm>
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int f = 8;
  int size = in.size();
  for (int i = 0; i < size - (f - 1); i += f) {
    for (int j = 0; j < size; j++) {
      for (int k = 0; k < f; k++) {
        out[i + k][j] = in[j][i + k];
      }
    }
  }
  int r = size % f;
  for (int i = size - r - 1; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
