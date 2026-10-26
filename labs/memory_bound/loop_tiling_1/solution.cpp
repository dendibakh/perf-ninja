#include "solution.hpp"
#include <algorithm>

// #ifdef SOLUTION
bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int height = in.size();
  int width = in[0].size();
  // root(l1cachesize / sizeof(double) / 2)
  static constexpr auto blockSize = 16;
  // iterating over blocks over width and height
  for (int bw = 0; bw < (width + blockSize - 1) / blockSize; ++bw) {
    for (int bh = 0; bh < (height + blockSize - 1) / blockSize; ++bh) {
      for (int i = bw * blockSize; i < std::min((bw + 1) * blockSize, width); ++i) {
        for (int j = bh * blockSize; j < std::min((bh + 1) * blockSize, height); ++j) {
          out[i][j] = in[j][i];
        }
      }
    }
  }
  return out[0][size - 1];
}
// #else
// bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
//   int size = in.size();
//   for (int i = 0; i < size; i++) {
//     for (int j = 0; j < size; j++) {
//       out[i][j] = in[j][i];
//     }
//   }
//   return out[0][size - 1];
// }
// #endif
