#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>


using simd_score_t = std::array<int16_t, sequence_count_v>;
using simd_sequence_t = std::array<simd_score_t, sequence_size_v>;



simd_sequence_t transpose(std::vector<sequence_t> const &sequences){
  /**
   * transpose the batch of sequences in order to have the sequence on column major,
   * thus contiguous in memory
  */

  assert(sequences.size() == sequence_count_v);
  
  simd_sequence_t simd_sequence{};
  size_t imax = simd_sequence.size();
  size_t jmax = sequences.size();

  for(size_t i=0; i<imax; ++i){
    for(size_t j=0; j<jmax; ++j){
      simd_sequence[i][j] = sequences[j][i];
    }
  }
  return simd_sequence;
}


// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2){

  
  result_t result{};
  
  auto seqs1_Tr = transpose(sequences1);
  auto seqs2_Tr = transpose(sequences2);

  using simd_column_t = std::array<simd_score_t, sequence_size_v+1>; // a column for every sequence
  
  /*
    * Initialise score values.
    */
  simd_score_t gap_open{};
  gap_open.fill(-11);
  simd_score_t gap_extension{};
  gap_extension.fill(-1);
  simd_score_t match{};
  match.fill(6);
  simd_score_t mismatch{};
  mismatch.fill(-4);

  /*
    * Setup the matrix.
    * Note we can compute the entire matrix with just one column in memory,
    * since we are only interested in the last value of the last column in the
    * score matrix.
    */
  simd_column_t score_column{};
  simd_column_t horizontal_gap_column{};
  simd_score_t last_vertical_gap{};

  /*
    * Initialise the first column of the matrix.
    */
  horizontal_gap_column[0] = gap_open;
  last_vertical_gap = gap_open;

  for (size_t i = 1; i < score_column.size(); ++i){
    for(size_t k=0; k<sequence_count_v; ++k){
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
      last_vertical_gap[k] += gap_extension[k];
    }
  }

  /*
    * Compute the main recursion to fill the matrix.
    */
  for(unsigned col=1; col<= seqs1_Tr.size(); ++col){
    simd_score_t last_diagonal_score = score_column[0];

    for(size_t k=0; k<sequence_count_v; ++k){
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
      horizontal_gap_column[0][k] += gap_extension[k];  
    }

    for(unsigned row=1; row<=seqs2_Tr.size(); ++row){
      simd_score_t best_cell_score = last_diagonal_score;
      for(size_t k=0; k<sequence_count_v; ++k){
        best_cell_score[k] += (seqs1_Tr[row-1][k] == seqs2_Tr[col-1][k] ? match[k] : mismatch[k]);
      }
      for(size_t k=0; k<sequence_count_v; ++k){
        best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);
        last_diagonal_score[k] = score_column[row][k];
        score_column[row][k] = best_cell_score[k];
        best_cell_score[k] += gap_open[k];
        last_vertical_gap[k] += gap_extension[k];
        horizontal_gap_column[row][k] += gap_extension[k];
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[row][k] = std::max(horizontal_gap_column[row][k], best_cell_score[k]);
      }

    }
  }

  for(size_t k=0; k<sequence_count_v; ++k){
    result[k] = score_column.back()[k];
  }


  return result;
}
