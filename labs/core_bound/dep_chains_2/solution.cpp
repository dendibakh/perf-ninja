#include "solution.hpp"

template<int N>
int process(const std::vector<Life::Grid>& grids, std::vector<int>& popCounts) {
  int i = 0;

  Life2<N> life2;
  for (; i + N - 1 < grids.size(); i += N) {

    std::vector<Life::Grid> subgrids;
    for (int j = 0; j < N; j++)
        subgrids.push_back(grids[i+j]);

    life2.reset(subgrids);
    for (int i = 0; i < NumberOfSims; i++) 
      life2.simulateTwoNext();

    for (int j = 0; j < N; j++)
      popCounts.push_back(life2.getPopulationCount(j));
  }
  return i;
}

// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids) {
  std::vector<int> popCounts;
  popCounts.reserve(grids.size());

  int i = process<2>(grids, popCounts); // 28% speedup
  //int i = process<3>(grids, popCounts); // 30% speedup
  //int i = process<4>(grids, popCounts); // 50% speedup
  //int i = process<5>(grids, popCounts); // 45% speedup
  //int i = process<6>(grids, popCounts); // 35% speedup
  //int i = process<7>(grids, popCounts); // 44% speedup
  //int i = process<8>(grids, popCounts); // 50% speedup
  //int i = process<16>(grids, popCounts);// 22% speedup
  
  Life life;
  for (; i < grids.size(); i++) {
    life.reset(grids[i]);
    for (int i = 0; i < NumberOfSims; i++)
      life.simulateNext();
    popCounts.push_back(life.getPopulationCount());
  }

  return popCounts;
}
