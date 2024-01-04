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
        Grid newGrid(grid.size() + 2, std::vector<int>(grid[0].size() + 2));

        int idx = 1;
        for (const auto &row : grid) {
          newGrid[idx].front() = newGrid[idx].back() = 0;
          std::copy(row.begin(), row.end(), newGrid[idx].begin() + 1);
          ++idx;
        }

        current = future = newGrid;
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
        const int M = current.size();
        const int N = current[0].size();

        // Loop through every cell
        for(int i = 1; i < M - 1; ++i) {
          for (int j = 1; j < N - 1; ++j) {
            // The cell needs to be subtracted from
            // its neighbours as it will be counted in the loop below
            int aliveNeighbours = -current[i][j];

            // finding the number of neighbours that are alive
            for (int p = -1; p <= 1; ++p) {   // row-offet (-1,0,1)
              for (int q = -1; q <= 1; ++q) { // col-offset (-1,0,1)
                const int di = i + p;
                const int dj = j + q;

                aliveNeighbours += current[di][dj];
              }
            }

            // Implementing the Rules of Life:
            if (__builtin_unpredictable(aliveNeighbours == 2)) {
              future[i][j] = current[i][j];
            }
            else {
              future[i][j] = aliveNeighbours == 3;
            }
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
