#include "solution.hpp"
#include <algorithm>


#ifndef SOLUTION
#  define SOLUTION 1
#endif

#if SOLUTION == 0
// Baseline.

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}

#elif SOLUTION == 1
// My solution.

bool solution(MatrixOfDoubles& in, MatrixOfDoubles& out)
{
  constexpr int TILE_SIZE {16};

  int size = in.size();
  for (int i_base = 0; i_base < size; i_base += TILE_SIZE)
  {
    for (int j_base = 0; j_base < size; j_base += TILE_SIZE)
    {
      for (int i_tile = i_base; i_tile < std::min(i_base + TILE_SIZE, size); ++i_tile)
      {
        for (int j_tile = j_base; j_tile < std::min(j_base + TILE_SIZE, size); ++j_tile)
        {
          out[i_tile][j_tile] = in[j_tile][i_tile];
        }
      }
    }
  }
  return out[0][size - 1];
}

#elif SOLUTION == 2
// Facit.

bool solution(MatrixOfDoubles& in, MatrixOfDoubles& out) {
  static constexpr int TILE_SIZE = 16;
  int size = in.size();
  for (int ii = 0; ii < size; ii+= TILE_SIZE) {
    for (int jj = 0; jj < size; jj+= TILE_SIZE) {
      for (int i = ii; i < std::min(ii + TILE_SIZE, size); i++) {
        for (int j = jj; j < std::min(jj + TILE_SIZE, size); j++) {
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif
