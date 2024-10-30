#include "solution.hpp"
#include <algorithm>
#include <cmath>

// #define SOLUTION

#ifdef SOLUTION

void CacheObliviousTransposition(int x, int delx, int y, int dely, int N,
                                 // MatrixOfDoubles const &in,
                                 MatrixOfDoubles &matrix) {
    if ((delx == 1) && (dely == 1)) {
        if(x<y)
        {
            // int tmp = matrix[(N*y) + x];
            // matrix[(N*y) + x] = matrix[(N*x) + y];
            // out[y][x] = in[x][y];
            // out[x][y] = in[y][x];
            auto tmp = matrix[y][x];
            matrix[y][x] = matrix[x][y];
            matrix[x][y] = tmp;
        }
        return;
    }

    if (delx >= dely) {
        int xmid = delx / 2;
        // CacheObliviousTransposition(x, xmid, y, dely, N, in, out);
        // CacheObliviousTransposition(x + xmid, delx - xmid, y, dely, N, in, out);
        CacheObliviousTransposition(x, xmid, y, dely, N, matrix);
        CacheObliviousTransposition(x + xmid, delx - xmid, y, dely, N, matrix);
        return;
    }

    int ymid = dely / 2;
    // CacheObliviousTransposition(x, delx, y, ymid, N, in, out);
    // CacheObliviousTransposition(x, delx, y + ymid, dely - ymid, N, in, out);
    CacheObliviousTransposition(x, delx, y, ymid, N, matrix);
    CacheObliviousTransposition(x, delx, y + ymid, dely - ymid, N, matrix);
}

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();

  // out = in;
  // CacheObliviousTransposition(0, size, 0, size, size, in, out);
  // CacheObliviousTransposition(0, size, 0, size, size, out);

  constexpr int tile_size = 32;
  for (int j = 0; j < size; j += tile_size) {
    for (int i = 0; i < size; i += tile_size) {

      for (int jj = j; jj < std::min(j+tile_size, size); jj++) {
        for (int ii = i; ii < std::min(i+tile_size, size); ii++) {
          out[ii][jj] = in[jj][ii];
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
