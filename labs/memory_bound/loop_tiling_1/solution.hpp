#include <vector>

constexpr int i_stride = 80;
constexpr int j_stride = 80;
using MatrixOfDoubles = std::vector<std::vector<double>>;
void initMatrix(MatrixOfDoubles& m);
bool solution(MatrixOfDoubles& in, MatrixOfDoubles& out);
