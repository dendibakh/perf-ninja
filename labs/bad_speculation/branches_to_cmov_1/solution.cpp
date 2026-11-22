#include "solution.hpp"

// Simulates N steps of the game for each starting grid 
// and return population count

// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bench1            782 ms          782 ms            1

//  Performance counter stats for './lab' (5 runs):

//         3690853846      cycles:u                                                                ( +-  0.29% )
//         7915725985      instructions:u                   #    2.14  insn per cycle              ( +-  0.00% )
//         1421213942      branches:u                                                              ( +-  0.00% )
//           44458140      branch-misses:u                  #    3.13% of all branches             ( +-  0.02% )

//            0.86108 +- 0.00335 seconds time elapsed  ( +-  0.39% )  

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