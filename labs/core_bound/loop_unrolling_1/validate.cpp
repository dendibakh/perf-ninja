
#include "solution.hpp"
#include <iostream>

std::vector<int> original_solution(const std::vector<Life::Grid>& grids) {
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

int main() {
  // Init benchmark data
  std::vector<Life::Grid> grids;
  for (int i = 0; i < NumberOfGrids; i++)
    grids.emplace_back(initRandom());

  auto original_result = original_solution(grids);
  auto result = solution(grids);
  
  if (original_result != result) {
    std::cerr << "Validation Failed. Population count doesn't match" << "\n";
    return 1;
  }

  std::cout << "Validation Successful" << std::endl;
  return 0;
}
