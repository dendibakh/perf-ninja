#include "solution.hpp"
#include <algorithm>


#if defined(__APPLE__) && defined(__MACH__)
  // In Apple processors (such as M1, M2 and later), L2 cache operates on 128B cache lines.
  #define ON_MACOS
  #define CACHELINE_SIZE 128
#else
  #define CACHELINE_SIZE 64
#endif

static constexpr int BLOCK_SIZE = CACHELINE_SIZE / sizeof(double);

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  for (int i = 0; i < size; i += BLOCK_SIZE) {
    for (int j = 0; j < size; j += BLOCK_SIZE) {
      for (int k = 0; k < BLOCK_SIZE; k++) {
        if (i + k >= size) continue;
        for (int l = 0; l < BLOCK_SIZE; l++) {
          if (j + l >= size) continue;
          out[i + k][j + l] = in[j + l][i + k];
        }
      }
    }
  }
  return out[0][size - 1];
}
