#include "solution.hpp"
#include <algorithm>
#include <random>

// Init random starting grid of the game
Life::Grid initRandom() {
  std::random_device r;
  std::mt19937_64 random_engine(r());
  std::uniform_int_distribution<int> distrib(0, 9);

  Life::Grid retGrid;
  retGrid.resize(GridXDimension);

  for (auto &row : retGrid) {
    row.resize(GridYDimension);
    std::generate(row.begin(), row.end(),[&]() { 
      // 70% dead cells and 30% alive cells
      return distrib(random_engine) > 6; 
    });
  }

  return retGrid;
}