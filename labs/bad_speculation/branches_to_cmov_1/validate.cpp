
#include "solution.hpp"
#include <iostream>

class LifeOriginal {

public:
    using Grid = std::vector<std::vector<int>>;
private:    
    Grid current;
    Grid future;

public:

    void reset(const Grid& grid) {
        current = future = grid;
    }

    int getPopulationCount() {
        int populationCount = 0;
        for (auto& row: current)
            for (auto& item: row)
                populationCount += item;
        return populationCount;
    }

    void printCurrentGrid() {
        for (auto& row: current) {
            for (auto& item: row)
                item ? std::cout << "x " : std::cout << ". ";
            std::cout << "\n";
        }
        std::cout << "\n";
    }    

    // Simulate the next generation of life
    void simulateNext() {
        //printCurrentGrid();
        int M = current.size();
        int N = current[0].size();
        
        // Loop through every cell
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                int aliveNeighbours = 0;      
                // finding the number of neighbours that are alive                  
                for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
                    for(int q = -1; q <= 1; q++) {          // col-offset (-1,0,1)
                        if((i + p < 0) ||                   // if row offset less than UPPER boundary
                           (i + p > M - 1) ||               // if row offset more than LOWER boundary
                           (j + q < 0) ||                   // if column offset less than LEFT boundary
                           (j + q > N - 1))                 // if column offset more than RIGHT boundary
                            continue;
                        aliveNeighbours += current[i + p][j + q];
                    }
                }
                // The cell needs to be subtracted from
                // its neighbours as it was counted before
                aliveNeighbours -= current[i][j];

                // Implementing the Rules of Life:
                switch(aliveNeighbours) {
                    // 1. Cell is lonely and dies
                    case 0:
                    case 1:
                        future[i][j] = 0;
                        break;                   
                    // 2. Remains the same
                    case 2:
                        future[i][j] = current[i][j];
                        break;
                    // 3. A new cell is born
                    case 3:
                        future[i][j] = 1;
                        break;
                    // 4. Cell dies due to over population
                    default:
                        future[i][j] = 0;
                }
            }
        }
        std::swap(current, future);
    }
};

std::vector<int> original_solution(const std::vector<LifeOriginal::Grid>& grids) {
  std::vector<int> popCounts;
  popCounts.reserve(grids.size());

  LifeOriginal life;
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
  std::vector<LifeOriginal::Grid> grids;
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
