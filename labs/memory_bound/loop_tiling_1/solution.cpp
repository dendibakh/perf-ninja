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
  constexpr int TILE_SIZE {2};

  int size = in.size();
  for (int i_base = 0; i_base < size - TILE_SIZE; i_base += TILE_SIZE)
  {
    for (int j_base = 0; j_base < size - TILE_SIZE; j_base += TILE_SIZE)
    {
      for (int i_tile = 0; i_tile < TILE_SIZE; ++i_tile)
      {
        int i = i_base + i_tile;
        for (int j_tile = 0; j_tile < TILE_SIZE; ++j_tile)
        {
          int j = j_base + j_tile;
          out[i][j] = in[j][i];
        }
      }
    }
    for (int i_tile = 0; i_tile < TILE_SIZE; ++i_tile)
    {
      int i = i_base + i_tile;
      for (int j = (size / TILE_SIZE) * TILE_SIZE; j < size; ++j)
      {
        out[i][j] = in[j][i];
      }
    }
  }
  for (int i = (size / TILE_SIZE) * TILE_SIZE; i < size; ++i)
  {
    for (int j = 0; j < size; ++j)
    {
      double in_v = in[j][i];
      double& out_v = out[i][j];
      out_v = in_v;
    }
  }
  return out[0][size - 1];
}

#elif SOLUTION == 2
// Facit.

// TODO Still using baseline code.
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}

#else
#  pragma error("Unknown solution. Valid values are 0 through 2.")
#endif
