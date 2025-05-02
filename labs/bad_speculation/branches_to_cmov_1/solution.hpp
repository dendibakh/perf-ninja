#include <variant>
#include <vector>
#include <iostream>

constexpr int NumberOfGrids = 16;
constexpr int GridXDimension = 1024;
constexpr int GridYDimension = 1024;
constexpr int NumberOfSims = 10;

class Life {

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

    void visitNeighboursEdges(int i, int j) {
      int M = current.size();
      int N = current[0].size();
      int aliveNeighbours = 0;
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

      aliveNeighbours -= current[i][j];

      // Implementing the Rules of Life:
      future[i][j] =
        __builtin_unpredictable(aliveNeighbours == 2) ?
          // 2. Remains the same
          current[i][j] :
          __builtin_unpredictable(aliveNeighbours == 3) ?
            // 3. A new cell is born
            1 :
            // 1. Cell is lonely and dies (or)
            // 4. Cell dies due to over population
            0;
    }

    // Simulate the next generation of life
    void simulateNext() {
        //printCurrentGrid();
        int M = current.size();
        int N = current[0].size();
        
        for(int i = 0; i < M; i++) {
          visitNeighboursEdges(i, 0);
          visitNeighboursEdges(i, N - 1);
        }
        for (int j = 1; j < N - 1; j++) {
          visitNeighboursEdges(0, j);
          visitNeighboursEdges(M - 1, j);
        }

        for(int i = 1; i < M - 1; i++) {
            for(int j = 1; j < N - 1; j++) {
                int aliveNeighbours = 0;

                for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
                  for(int q = -1; q <= 1; q++) {           // col-offset (-1,0,1)
                    aliveNeighbours += current[i + p][j + q];
                  }
                }
                // finding the number of neighbours that are alive                  
                // The cell needs to be subtracted from
                // its neighbours as it was counted before
                aliveNeighbours -= current[i][j];

                // Implementing the Rules of Life:
                future[i][j] =
                  __builtin_unpredictable(aliveNeighbours == 2) ?
                    // 2. Remains the same
                    current[i][j] :
                    __builtin_unpredictable(aliveNeighbours == 3) ?
                      // 3. A new cell is born
                      1 :
                      // 1. Cell is lonely and dies (or)
                      // 4. Cell dies due to over population
                      0;
            }
        }

        std::swap(current, future);
    }
};

// Init random starting grid of the game
Life::Grid initRandom();
// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids);
