#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  const size_t size = in.size();

  constexpr size_t kStride = 40;

  for (size_t i = 0; i < size; i += kStride) {
    for (size_t j = 0; j < size; j += kStride) {
      for (size_t l = j; l < std::min(size, j+kStride); l++) {
        for (size_t k = i; k < std::min(size, i+kStride); k++) {
          out[l][k] = in[k][l];
        }
      }
    }
  }

  return out[0][size - 1];
}
