


#include <array>
#include <vector>
#include <iostream>
#include <cassert>

constexpr int NumberOfGrids = 16;
constexpr int GridXDimension = 1025;
constexpr int GridYDimension = 1025;
constexpr int NumberOfSims = 10;


const std::array<int, 8> dx{-1, 0, 1, -1, 1,-1, 0, 1};
const std::array<int, 8> dy{-1, -1, -1, 0, 0, 1, 1, 1};

const std::array<std::array<int, 9>, 2> transition{{
    {0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0, 0}
}};

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

    // Simulate the next generation of life
    void simulateNext() {
        //printCurrentGrid();
        int M = current.size();
        int N = current[0].size();

        // Loop through every cell
        for(int i = 0; i < M; ++i) {
            for(int j = 0; j < N; ++j) {
                int aliveNeighbours = 0;
                for (int d = 0; d < 8; ++d) {
                    const int ny = i + dy[d];
                    const int nx = j + dx[d];


                    //aliveNeighbours += ((ny >= 0) * (ny < M) * (nx >= 0) * (nx < N)) and  current[ny][nx];
                     //aliveNeighbours += (ny >= 0 and ny < M and nx >= 0 and nx < N and current[ny][nx]);
                    if (ny >= 0 and ny < M and nx >= 0 and nx < N) {
                        aliveNeighbours+=current[ny][nx];
                    }
                }
                future[i][j] = transition[current[i][j]][aliveNeighbours];
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
