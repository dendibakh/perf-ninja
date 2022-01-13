#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  constexpr int CACHE_LINE_SIZE = 64;
  constexpr int ELMNS_PER_CL = CACHE_LINE_SIZE / sizeof(MatrixOfDoubles::value_type::value_type);
  constexpr int HORIZONTAL_STRIP = 6 * ELMNS_PER_CL;
  constexpr int VERTICAL_STRIP = HORIZONTAL_STRIP;

  const int size = static_cast<int>(in.size());

  for (int ii = 0; ii < size; ii += HORIZONTAL_STRIP) {

    for (int jj = 0; jj < size; jj += VERTICAL_STRIP) {

      for (int i = ii; i < std::min(size, ii + HORIZONTAL_STRIP); ++i) {

        for (int j = jj; j < std::min(size, jj + VERTICAL_STRIP); ++j) {
          out[i][j] = in[j][i];
        }

      }

    }

  }

  return out.front().back();
}
