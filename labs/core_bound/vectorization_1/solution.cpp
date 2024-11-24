#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

using row_vector_t = std::array<int16_t, sequence_count_v>;
using sequences_matrix_t = std::array<row_vector_t, sequence_size_v>;

sequences_matrix_t transpose(std::vector<sequence_t> const &sequences)
{
  sequences_matrix_t sequences_matrix{};
  for (size_t i = 0; i < sequence_size_v; i++)
    for (size_t j = 0; j < sequence_count_v; j++) 
      sequences_matrix[i][j] = sequences[j][i];
  return sequences_matrix;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  sequences_matrix_t seq1 = transpose(sequences1);
  sequences_matrix_t seq2 = transpose(sequences2);


  using score_t = row_vector_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  /*
    * Initialise score values.
    */
  score_t gap_open{};
  gap_open.fill(-11);
  score_t gap_extension{};
  gap_extension.fill(-1);
  score_t match{};
  match.fill(6);
  score_t mismatch{};
  mismatch.fill(-4);

  /*
    * Setup the matrix.
    * Note we can compute the entire matrix with just one column in memory,
    * since we are only interested in the last value of the last column in the
    * score matrix.
    */
  column_t score_column{};
  column_t horizontal_gap_column{};
  score_t last_vertical_gap{};

  /*
    * Initialise the first column of the matrix.
    */
  horizontal_gap_column[0] = gap_open;
  last_vertical_gap = gap_open;

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t s = 0; s < sequence_count_v; s++) {
      score_column[i][s] = last_vertical_gap[s];
      horizontal_gap_column[i][s] = last_vertical_gap[s] + gap_open[s];
      last_vertical_gap[s] += gap_extension[s];
    }
  }

  /*
    * Compute the main recursion to fill the matrix.
    */
  for (unsigned col = 1; col <= seq2.size(); ++col) {
    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.

    
    for (size_t s = 0; s < sequence_count_v; s++) {
      score_column[0][s] = horizontal_gap_column[0][s];
      last_vertical_gap[s] = horizontal_gap_column[0][s] + gap_open[s];
      horizontal_gap_column[0][s] += gap_extension[s];
    }

    for (unsigned row = 1; row <= seq1.size(); ++row) {
      // Compute next score from diagonal direction with match/mismatch.
      score_t best_cell_score = last_diagonal_score;
      for (size_t s = 0; s < sequence_count_v; s++) {
        best_cell_score[s] += (seq1[row - 1][s] == seq2[col - 1][s] ? match[s] : mismatch[s]);
      }
      for (size_t s = 0; s < sequence_count_v; s++) {
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[s] = std::max(best_cell_score[s], last_vertical_gap[s]);
        best_cell_score[s] = std::max(best_cell_score[s], horizontal_gap_column[row][s]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[s] = score_column[row][s];
        score_column[row][s] = best_cell_score[s];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[s] += gap_open[s];
        last_vertical_gap[s] += gap_extension[s];
        horizontal_gap_column[row][s] += gap_extension[s];
        // Store optimum between gap open and gap extension.
        last_vertical_gap[s] = std::max(last_vertical_gap[s], best_cell_score[s]);
        horizontal_gap_column[row][s] = std::max(horizontal_gap_column[row][s], best_cell_score[s]);
      }
    }
  }

  for (size_t s = 0; s < sequence_count_v; s++) {
    // Report the best score.
    result[s] = score_column.back()[s];
  }

  return result;
}
