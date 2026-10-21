#include <vector>

using MatrixOfDoubles = std::vector<std::vector<double>>;
void initMatrix(MatrixOfDoubles& m);
bool solution(MatrixOfDoubles& in, MatrixOfDoubles& out);
bool solution_fix(MatrixOfDoubles& in, MatrixOfDoubles& out, int TILE_SIZE);
