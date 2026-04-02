#include <vector>
#include <iostream>

constexpr int NumberOfGrids = 16;
constexpr int GridXDimension = 1024;
constexpr int GridYDimension = 1024;
constexpr int NumberOfSims = 10;

class Life
{

public:
    using Grid = std::vector<std::vector<int>>;

private:
    Grid current;
    Grid future;
    int M, N;

public:
    void reset(const Grid &grid)
    {
        M = grid.size();
        N = grid[0].size();
        current.assign(M + 2, std::vector<int>(N + 2, 0));
        future.assign(M + 2, std::vector<int>(N + 2, 0));
        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++)
                current[i + 1][j + 1] = grid[i][j];
    }

    int getPopulationCount()
    {
        int populationCount = 0;
        for (int i = 1; i <= M; i++)
            for (int j = 1; j <= N; j++)
                populationCount += current[i][j];
        return populationCount;
    }

    void printCurrentGrid()
    {
        for (int i = 1; i <= M; i++)
        {
            for (int j = 1; j <= N; j++)
                current[i][j] ? std::cout << "x " : std::cout << ". ";
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    void simulateNext()
    {

        static constexpr int LUT[2][9] = {
            // 0  1  2  3  4  5  6  7  8  (Neighbors)
            {0, 0, 0, 1, 0, 0, 0, 0, 0}, // Dead cells only come alive at exactly 3
            {0, 0, 1, 1, 0, 0, 0, 0, 0}  // Alive cells survive at 2 or 3
        };

        for (int i = 1; i <= M; i++)
        {
            for (int j = 1; j <= N; j++)
            {
                int aliveNeighbours = 0;
                for (int p = -1; p <= 1; p++)
                {
                    for (int q = -1; q <= 1; q++)
                    {
                        aliveNeighbours += current[i + p][j + q];
                    }
                }
                auto self = current[i][j];
                aliveNeighbours -= self;

                future[i][j] = LUT[self][aliveNeighbours];
            }
        }
        std::swap(current, future);
    }
};

Life::Grid initRandom();
std::vector<int> solution(const std::vector<Life::Grid> &grids);
