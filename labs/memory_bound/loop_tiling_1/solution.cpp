#include "solution.hpp"
#include <algorithm>


#ifdef SOLUTION
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  constexpr int stride = 16;

  for(int x = 0; x < size; x+=stride)
  {
    for(int y = 0; y < size; y+=stride)
    {
      for (int i = x; i < std::min(size, x+stride); i++) 
      {
        for (int j = y; j < std::min(size, y+stride); j++) 
        {
          out[i][j] = in[j][i];
        }
      }
    }
  }

  return out[0][size - 1];
}
#else
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      out[i][j] = in[j][i];
    }
  }
  return out[0][size - 1];
}
#endif
