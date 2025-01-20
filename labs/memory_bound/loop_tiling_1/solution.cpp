#include "solution.hpp"
#include <algorithm>

#define SOLUTION
#ifdef SOLUTION
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out)
{
  constexpr int tile_size = 16;
  const int size = in.size();
  for (int ii = 0; ii < size; ii += tile_size)
  {
    for (int jj = 0; jj < size; jj += tile_size)
    {
      for (int i = ii; i < std::min(size, ii + tile_size); i++)
      {
        for (int j = jj; j < std::min(size, jj + tile_size); j++)
        {
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}
#else
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out)
{
  int size = in.size();
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < size; j++)
    {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
#endif