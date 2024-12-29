#include "solution.hpp"
#include <algorithm>

bool solution(MatrixOfDoubles &in, MatrixOfDoubles &out) {
  int size = in.size();
  int tile_size = 128;
  for (int i=0; i < size; i+=tile_size){
    for (int j = 0; j < size; j+=tile_size) {

      for(int t_i=0; t_i < tile_size && i+t_i<size; ++t_i){
        for(int t_j=0; t_j < tile_size && j+t_j<size; ++t_j){

          out[i+t_i][j+t_j] = in[j+t_j][i+t_i];

        }
      }

    }
  }
  return out[0][size - 1];
}
