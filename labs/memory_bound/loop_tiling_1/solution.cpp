#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size/2; i++) {
    for (int j = 0; j < size/2; j++) {
      out[i][j] = in[j][i];
    }
  }
    for (int i = 0; i < size/2; i++) {
    for (int j = size/2; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
    for (int i = size/2; i < size; i++) {
    for (int j = 0; j < size/2; j++) {
      out[i][j] = in[j][i];
    }
  }
    for (int i = size/2; i < size; i++) {
    for (int j = size/2; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
