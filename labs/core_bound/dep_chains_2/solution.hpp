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

template <int N>
class Life2 {

public:
    using Grid = std::vector<std::vector<int>>;
private:    
    Grid current[N];
    Grid future[N];

public:

    void reset(const std::vector<Grid>& grids) {
        for (int i = 0; i < N; i++)
            current[i] = future[i] = grids[i];
    }

    int getPopulationCount(int L) {
        int populationCount = 0;
        for (auto& row: current[L])
            for (auto& item: row)
                populationCount += item;
        return populationCount;
    }

    void printCurrentGrid(int L) {
        for (auto& row: current[L]) {
            for (auto& item: row)
                item ? std::cout << "x " : std::cout << ". ";
            std::cout << "\n";
        }
        std::cout << "\n";
    }    

    // Simulate the next generation of life
    void simulateTwoNext() {
        //printCurrentGrid();
        int M = current[0].size();
        int K = current[0][0].size();

        // Loop through every cell
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < K; j++) {
/*                
                int aliveNeighbours0 = 0;      
                int aliveNeighbours1 = 0;      
                // finding the number of neighbours that are alive                  
                for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
                    for(int q = -1; q <= 1; q++) {          // col-offset (-1,0,1)
                        if((i + p < 0) ||                   // if row offset less than UPPER boundary
                           (i + p > M - 1) ||               // if row offset more than LOWER boundary
                           (j + q < 0) ||                   // if column offset less than LEFT boundary
                           (j + q > K - 1))                 // if column offset more than RIGHT boundary
                            continue;
                        aliveNeighbours0 += current[0][i + p][j + q];
                        aliveNeighbours1 += current[1][i + p][j + q];
                    }
                }
                // The cell needs to be subtracted from
                // its neighbours as it was counted before
                aliveNeighbours0 -= current[0][i][j];
                aliveNeighbours1 -= current[1][i][j];
*/
                auto calcNeighbors = [this, i, j, K, M](int L) {
                    int aliveNeighbours = 0;      
                    // finding the number of neighbours that are alive                  
                    for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
                        for(int q = -1; q <= 1; q++) {          // col-offset (-1,0,1)
                            if((i + p < 0) ||                   // if row offset less than UPPER boundary
                            (i + p > M - 1) ||               // if row offset more than LOWER boundary
                            (j + q < 0) ||                   // if column offset less than LEFT boundary
                            (j + q > K - 1))                 // if column offset more than RIGHT boundary
                                continue;
                            aliveNeighbours += current[L][i + p][j + q];
                        }
                    }
                    // The cell needs to be subtracted from
                    // its neighbours as it was counted before
                    aliveNeighbours -= current[L][i][j];
                    return aliveNeighbours;
                };

                auto calcFutureGrid = [this, i, j](int L, int aliveNeighbours) {
                    // Implementing the Rules of Life:
                    switch(aliveNeighbours) {
                        // 1. Cell is lonely and dies
                        case 0:
                        case 1:
                            future[L][i][j] = 0;
                            break;                   
                        // 2. Remains the same
                        case 2:
                            future[L][i][j] = current[L][i][j];
                            break;
                        // 3. A new cell is born
                        case 3:
                            future[L][i][j] = 1;
                            break;
                        // 4. Cell dies due to over population
                        default:
                            future[L][i][j] = 0;
                    }
                };

                int aliveNeighbours[N];
                for (int i = 0; i < N; i++)
                    aliveNeighbours[i] = calcNeighbors(i);

                for (int i = 0; i < N; i++)
                    calcFutureGrid(i, aliveNeighbours[i]);
            }
        }
        for (int i = 0; i < N; i++)
            std::swap(current[i], future[i]);
    }
};

// Init random starting grid of the game
Life::Grid initRandom();
// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids);
