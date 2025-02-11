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
			int i = 0;
			int j = 0;
			int aliveNeighbours = 0;      
			// finding the number of neighbours that are alive                  
			for(int p = 0; p <= 1; p++) {              // row-offet (-1,0,1)
				for(int q = 0; q <= 1; q++) {          // col-offset (-1,0,1)
					aliveNeighbours += current[i + p][j + q];
				}
			}
			// The cell needs to be subtracted from
			// its neighbours as it was counted before
			aliveNeighbours -= current[i][j];

			// Implementing the Rules of Life:
			future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;

			for(j = 1; j < N-1; j++) {
				aliveNeighbours = 0;      
				// finding the number of neighbours that are alive                  
				for(int p = 0; p <= 1; p++) {              // row-offet (-1,0,1)
					for(int q = -1; q <= 1; q++) {          // col-offset (-1,0,1)
						aliveNeighbours += current[i + p][j + q];
					}
				}
				// The cell needs to be subtracted from
				// its neighbours as it was counted before
				aliveNeighbours -= current[i][j];

				// Implementing the Rules of Life:
				future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;
			}
			j = N - 1;
			aliveNeighbours = 0;      
			// finding the number of neighbours that are alive                  
			for(int p = 0; p <= 1; p++) {              // row-offet (-1,0,1)
				for(int q = -1; q <= 0; q++) {          // col-offset (-1,0,1)
					aliveNeighbours += current[i + p][j + q];
				}
			}
			// The cell needs to be subtracted from
			// its neighbours as it was counted before
			aliveNeighbours -= current[i][j];

			// Implementing the Rules of Life:
			future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;


			for(i = 1; i < M-1; i++) {
				j = 0;
				aliveNeighbours = 0;      
				// finding the number of neighbours that are alive                  
				for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
					for(int q = 0; q <= 1; q++) {          // col-offset (-1,0,1)
						aliveNeighbours += current[i + p][j + q];
					}
				}
				// The cell needs to be subtracted from
				// its neighbours as it was counted before
				aliveNeighbours -= current[i][j];

				// Implementing the Rules of Life:
				future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;

				for(j = 1; j < N-1; j++) {
					aliveNeighbours = 0;      
					// finding the number of neighbours that are alive                  
					for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
						for(int q = -1; q <= 1; q++) {          // col-offset (-1,0,1)
							aliveNeighbours += current[i + p][j + q];
						}
					}
					// The cell needs to be subtracted from
					// its neighbours as it was counted before
					aliveNeighbours -= current[i][j];

					// Implementing the Rules of Life:
					future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;
				}
				j = N - 1;
				aliveNeighbours = 0;      
				// finding the number of neighbours that are alive                  
				for(int p = -1; p <= 1; p++) {              // row-offet (-1,0,1)
					for(int q = -1; q <= 0; q++) {          // col-offset (-1,0,1)
						aliveNeighbours += current[i + p][j + q];
					}
				}
				// The cell needs to be subtracted from
				// its neighbours as it was counted before
				aliveNeighbours -= current[i][j];

				// Implementing the Rules of Life:
				future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;

			}
			i = M-1;
			j = 0;
			aliveNeighbours = 0;      
			// finding the number of neighbours that are alive                  
			for(int p = -1; p <= 0; p++) {              // row-offet (-1,0,1)
				for(int q = 0; q <= 1; q++) {          // col-offset (-1,0,1)
					aliveNeighbours += current[i + p][j + q];
				}
			}
			// The cell needs to be subtracted from
			// its neighbours as it was counted before
			aliveNeighbours -= current[i][j];

			// Implementing the Rules of Life:
			future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;

			for(j = 1; j < N-1; j++) {
				aliveNeighbours = 0;      
				// finding the number of neighbours that are alive                  
				for(int p = -1; p <= 0; p++) {              // row-offet (-1,0,1)
					for(int q = -1; q <= 1; q++) {          // col-offset (-1,0,1)
						aliveNeighbours += current[i + p][j + q];
					}
				}
				// The cell needs to be subtracted from
				// its neighbours as it was counted before
				aliveNeighbours -= current[i][j];

				// Implementing the Rules of Life:
				future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;
			}
			j = N - 1;
			aliveNeighbours = 0;      
			// finding the number of neighbours that are alive                  
			for(int p = -1; p <= 0; p++) {              // row-offet (-1,0,1)
				for(int q = -1; q <= 0; q++) {          // col-offset (-1,0,1)
					aliveNeighbours += current[i + p][j + q];
				}
			}
			// The cell needs to be subtracted from
			// its neighbours as it was counted before
			aliveNeighbours -= current[i][j];

			// Implementing the Rules of Life:
			future[i][j] = 1 < ((unsigned)aliveNeighbours - 2) ? 0 : aliveNeighbours == 2 ? current[i][j] : 1;


			std::swap(current, future);
		}
};

// Init random starting grid of the game
Life::Grid initRandom();
// Simulates N steps of the game for each starting grid 
// and return population count
std::vector<int> solution(const std::vector<Life::Grid>& grids);
