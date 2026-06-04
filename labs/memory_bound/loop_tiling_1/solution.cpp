#include "solution.hpp"
#include <algorithm>
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size - 1; i+=2) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
      out[i + 1][j] = in[j][i + 1];
    }
  }
  if (size % 2 == 1) {
    for (int j = 0; j < size; j++) {
      out[size - 1][j] = in[j][size -1];
    }
  }
  return out[0][size - 1];
}
