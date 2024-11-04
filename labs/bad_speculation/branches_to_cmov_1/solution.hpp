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

#define SOLUTION
  // __attribute__((noinline))
  void predictFuture(int i, int j, int aliveNeighbours) {
    #ifdef SOLUTION
    int fut = current[i][j];
    if (__builtin_unpredictable(aliveNeighbours == 3)) {
      fut = 1;
    }
    if (__builtin_unpredictable(aliveNeighbours < 2 || aliveNeighbours >= 4)) {
      fut = 0;
    }
    future[i][j] = fut;
    #else
    switch(aliveNeighbors) {
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
    #endif
  }


    // Simulate the next generation of life
  // __attribute__((noinline))
    void simulateNext() {
        //printCurrentGrid();
        int M = current.size();
        int N = current[0].size();
        
#ifdef SOLUTION
        {
          int i = 0, j = 0;
          int aliveNeighbours = current[i+1][j] + current[i][j+1] + current[i+1][j+1];
          predictFuture(i, j, aliveNeighbours);
        }
        {
          int i = M-1, j = 0;
          int aliveNeighbours = current[i-1][j] + current[i][j+1] + current[i-1][j+1];
          predictFuture(i, j, aliveNeighbours);
        }
        {
          int i = 0, j = N-1;
          int aliveNeighbours = current[i+1][j] + current[i][j-1] + current[i+1][j-1];
          predictFuture(i, j, aliveNeighbours);
        }
        {
          int i = M-1, j = N-1;
          int aliveNeighbours = current[i-1][j] + current[i][j-1] + current[i-1][j-1];
          predictFuture(i, j, aliveNeighbours);
        }
        for (int i = 1; i < M-1; i++) {
          {
            int j = 0;
            int aliveNeighbours = current[i + 1][j] + current[i + 1][j + 1] + current[i][j + 1] + current[i - 1][j + 1] + current[i - 1][j];
            predictFuture(i, j, aliveNeighbours);
          }
          {
            int j = N-1;
            int aliveNeighbours = current[i + 1][j] + current[i + 1][j - 1] + current[i][j - 1] + current[i - 1][j - 1] + current[i - 1][j];
            predictFuture(i, j, aliveNeighbours);
          }
        }
        for (int j = 1; j < N-1; j++) {
          {
            int i = 0;
            int aliveNeighbours = current[i+1][j] + current[i+1][j+1] + current[i][j+1] + current[i][j-1] + current[i+1][j-1];
            predictFuture(i, j, aliveNeighbours);
          }
          {
            int i = M-1;
            int aliveNeighbours = current[i-1][j] + current[i-1][j+1] + current[i][j+1] + current[i][j-1] + current[i-1][j-1];
            predictFuture(i, j, aliveNeighbours);
          }
        }
        for(int i = 1; i < M-1; i++) {
          for(int j = 1; j < N-1; j++) {
            int aliveNeighbours = current[i-1][j-1] + current[i-1][j] + current[i-1][j+1] +
                                  current[i  ][j-1] + /*current[i][j] +*/ current[i  ][j+1] +
                                  current[i+1][j-1] + current[i+1][j] + current[i+1][j+1];
            predictFuture(i, j, aliveNeighbours);
          }
        }

#else
        for(int i = 0; i < M; i++) {
          for(int j = 0; j < N; j++) {
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
            predictFuture(i, j, aliveNeighbours);
          }
        }
#endif
        std::swap(current, future);
    }
};

// Init random starting grid of the game
Life::Grid initRandom();
// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids);
