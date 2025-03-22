#include "solution.hpp"
#include <algorithm>
#include <cstdio>

#define SOLUTION

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
#ifdef SOLUTION
  // that tile size gives best perfomance on my architecture
  // with greater tile size we get cache misses
  // with less - donot fully use existing cache
  // I guess with tile switch cache is invalidated.
  // so with increased amount of tiles more invalidations happen
  // that's why perfomance goes down
  /*
    3.32 ms (TileSize = 9)
    3.18 ms (TileSize = 8)
    3.48 ms (TileSize = 7)

    by this data we can say that on this dataset
    invalidation is more crucial than cache miss
  */
  constexpr int TileSize = 8;
  int size = in.size();

  int tileNum= size / TileSize;
  int tail = size % TileSize;

  // Process in tiles
  for (int tileY = 0; tileY < tileNum; tileY++) {
    for (int tileX = 0; tileX < tileNum; tileX++) {
      int xStart = tileX * TileSize;
      int yStart = tileY * TileSize;

      // Process one tile
      for (int x = 0; x < TileSize; x++) {
        for (int y = 0; y < TileSize; y++) {
          int i = xStart + x;
          int j = yStart + y;
          // printf("i = %d, j = %d\n", i, j);
          out[i][j] = in[j][i];
        }
      }
    }
  }

  // Process tale
  int st = tileNum * TileSize;
  for (int y = 0; y < size-tail; y++) {
    for (int x = st; x < size; x++) {
      out[y][x] = in[x][y];
    }
  }
  
  for (int y = st; y < size; y++) {
    for (int x = 0; x < size; x++) {
      out[y][x] = in[x][y];
    }
  }
#else
  auto size = in.size();
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      out[i][j] = in[j][i];

  return out[0][size - 1];
#endif

  return out[0][size - 1];
}
