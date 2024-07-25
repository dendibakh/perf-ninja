#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  static constexpr auto tillingSize = 16;
  int size = in.size();

  for (int i = 0; i < size; i+=tillingSize) {
    for (int j = 0; j < size; j+=tillingSize) {
      for(int k = i; k < std::min(i + tillingSize, size); ++k) {
        for(int l = j; l < std::min(j + tillingSize, size); ++l)
          out[k][l] = in[l][k];
      }
    }
  }
  return out[0][size - 1];
}
