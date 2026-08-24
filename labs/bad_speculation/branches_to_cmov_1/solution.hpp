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
        current = Grid(grid.size() + 2, std::vector<int>(grid[0].size() + 2, 0));
        for (size_t i = 0; i < grid.size(); ++i) {
            for (size_t j = 0; j < grid[0].size(); ++j) {
                current[i + 1][j + 1] = grid[i][j];
            }
        }
        future = current;
    }

    int getPopulationCount() {
        int populationCount = 0;
        for (auto& row: current)
            for (auto& item: row)
                populationCount += item;
        return populationCount;
    }

    void printCurrentGrid() {
        for (size_t i = 1; i <= current.size() - 2; ++i) {
            for (size_t j = 1; j <= current[i].size() - 2; ++j) {
                current[i][j] ? std::cout << "x " : std::cout << ". ";
            }
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
        for(int i = 1; i <= M - 2; i++) {
            for(int j = 1; j <= N - 2; j++) {
                int aliveNeighbours = 0;
                // finding the number of neighbours that are alive
                for(int p = -1; p <= 1; p++) {
                    for(int q = -1; q <= 1; q++) {
                        aliveNeighbours += current[i + p][j + q];
                    }
                }
                // The cell needs to be subtracted from
                // its neighbours as it was counted before
                aliveNeighbours -= current[i][j];

                future[i][j] = ((aliveNeighbours ^ 2) | current[i][j]) == 1 ? 1 : 0;
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
