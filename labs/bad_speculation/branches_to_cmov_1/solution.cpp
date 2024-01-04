#include "solution.hpp"

// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids) {
  std::vector<int> popCounts;
  popCounts.reserve(grids.size());

  Life life;
  for (auto& grid : grids) {
    life.reset(grid);
    for (int i = 0; i < NumberOfSims; i++)
      life.simulateNext();
    popCounts.push_back(life.getPopulationCount());
  }

  return popCounts;
}