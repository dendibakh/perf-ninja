#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

// Introduce a vector of scores.
using simd_score_t = std::array<int16_t, sequence_count_v>;
using simd_sequence_t = std::array<simd_score_t, sequence_size_v>;

// Transpose a collection of sequences to a sequence of simd vectors.
simd_sequence_t transpose(std::vector<sequence_t> const &sequences) {
  assert(sequences.size() == sequence_count_v);

  simd_sequence_t simd_sequence{};

  for (size_t i = 0; i < simd_sequence.size(); ++i)
    for (size_t j = 0; j < sequences.size(); ++j)
      simd_sequence[i][j] = sequences[j][i];

  return simd_sequence;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  // Transpose the sequences.
  auto trSeq1 = transpose(sequences1);
  auto trSeq2 = transpose(sequences2);

  using score_t = simd_score_t;
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
    for (size_t k = 0; k < sequence_count_v; ++k) {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
      last_vertical_gap[k] += gap_extension[k];
    }
  }

  /*
    * Compute the main recursion to fill the matrix.
    */
  for (unsigned col = 1; col <= trSeq2.size(); ++col) {
    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.
    for (size_t k = 0; k < sequence_count_v; ++k) {
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
      horizontal_gap_column[0][k] += gap_extension[k];
    }

    for (unsigned row = 1; row <= trSeq1.size(); ++row) {
      // Compute next score from diagonal direction with match/mismatch.
      score_t best_cell_score = last_diagonal_score;
      for (size_t k = 0; k < sequence_count_v; ++k)
        best_cell_score[k] += (trSeq1[row - 1][k] == trSeq2[col - 1][k] ? match[k] : mismatch[k]);
      for (size_t k = 0; k < sequence_count_v; ++k) {
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[k] = score_column[row][k];
        score_column[row][k] = best_cell_score[k];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[k] += gap_open[k];
        last_vertical_gap[k] += gap_extension[k];
        horizontal_gap_column[row][k] += gap_extension[k];
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[row][k] =
            std::max(horizontal_gap_column[row][k], best_cell_score[k]);
      }
    }
  }

  // Report the best score.
  for (size_t k = 0; k < sequence_count_v; ++k)
    result[k] = score_column.back()[k];

  return result;
}
