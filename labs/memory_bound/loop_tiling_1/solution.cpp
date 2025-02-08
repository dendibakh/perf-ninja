#include "solution.hpp"
#include <algorithm>
#include <cstdio>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  constexpr int TILE_SIZE = 64;
  const int sizeInTiles = (size + TILE_SIZE - 1) / TILE_SIZE;
  for (int tileY = 0; tileY < sizeInTiles; ++tileY) {
    for (int tileX = 0; tileX < sizeInTiles; ++tileX) {
      for (int y = 0; y < TILE_SIZE; ++y) {
        const int finalY = tileY * TILE_SIZE + y;
        if (finalY == size) {
          break;
        }
        for (int x = 0; x < TILE_SIZE; ++x) {
          const int finalX = tileX * TILE_SIZE + x;
          if (finalX == size) {
            break;
          }
          out[finalY][finalX] = in[finalX][finalY];
        }
      }
    }
  }
  return out[0][size - 1];
}
