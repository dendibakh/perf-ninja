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
        for (auto& row: current) {
            #pragma unroll
            for (std::size_t i = 0; i < row.size(); i++) {
                populationCount += row[i];
            }
        }
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
        
        auto step_fn = [&](int i, int j, int pmin, int pmax, int qmin, int qmax) {
            int aliveNeighbours = 0;      
            // finding the number of neighbours that are alive                  
            for(int p = pmin; p <= pmax; p++) {              // row-offet (-1,0,1)
                for(int q = qmin; q <= qmax; q++) {          // col-offset (-1,0,1)
                    aliveNeighbours += current[i + p][j + q];
                }
            }
            // The cell needs to be subtracted from
            // its neighbours as it was counted before
            aliveNeighbours -= current[i][j];

            // Implementing the Rules of Life:
            future[i][j] = int(aliveNeighbours == 2) * current[i][j]+ aliveNeighbours == 3;
        };

        // Loop through every cell
        for(int i = 1; i < M - 1; i++) {
            for(int j = 1; j < N - 1; j++) {
                step_fn(i, j, -1, 1, -1, 1);
            }
        }

        for (int i = 1; i < M - 1; i++) {
            step_fn(i, 0, -1, 1, 0, 1);
            step_fn(i, N - 1, -1, 1, -1, 0);
        }

        for (int j = 1; j < N - 1; j++) {
            step_fn(0, j, 0, 1, -1, 1);
            step_fn(M - 1, j, -1, 0, -1, 1);
        }

        step_fn(0, 0, 0, 1, 0, 1);
        step_fn(0, N - 1, 0, 1, -1, 0);
        step_fn(M - 1, 0, -1, 0, 0, 1);
        step_fn(M - 1, N - 1, -1, 0, -1, 0);

        std::swap(current, future);
    }
};

// Init random starting grid of the game
Life::Grid initRandom();
// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids);
