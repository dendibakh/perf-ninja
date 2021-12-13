#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  size_t size = in.size();

  constexpr size_t kStride = 4;

  const size_t full_strides = size - size % kStride;

  for (size_t i = 0; i < full_strides; i += kStride) {
    for (size_t j = 0; j < full_strides; j += kStride) {
      for (size_t k = 0; k < kStride; k++) {
        for (size_t l = 0; l < kStride; l++) {
          out[j + l][i + k] = in[i + k][j + l];
        }
      }
    }
  }
  for (size_t i = full_strides; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  for (size_t i = 0; i < size; i++) {
    for (size_t j = full_strides; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }

  return out[0][size - 1];
}
