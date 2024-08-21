#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>

// The alignment algorithm which computes the alignment of the given sequence
// pairs.

using transparent_matrix = std::array<std::array<int16_t, sequence_count_v>, sequence_size_v>;

transparent_matrix produce_transperent(const std::vector<sequence_t>& matrix) {
  transparent_matrix result;

  for (size_t i = 0; i < sequence_size_v; ++i) {
    for (size_t j = 0; j < sequence_count_v; ++j) {
      result[i][j] = matrix[j][i];
    }
  }

  return result;
}

result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                          std::vector<sequence_t> const &sequences2) {
  result_t result{};

  auto sequences1_t = produce_transperent(sequences1);
  auto sequences2_t = produce_transperent(sequences2);

  using score_t = std::array<int16_t, sequence_count_v>;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  /*
    * Initialise score values.
    */
  score_t gap_open; 
  std::fill(gap_open.begin(), gap_open.end(), -11);
  score_t gap_extension;
  std::fill(gap_extension.begin(), gap_extension.end(), -1);
  score_t match;
  std::fill(match.begin(), match.end(), 6);
  score_t mismatch;
  std::fill(mismatch.begin(), mismatch.end(), -4);

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
    for (size_t j = 0; j < sequence_count_v; ++j) {
      score_column[i][j] = last_vertical_gap[j];
      horizontal_gap_column[i][j] = last_vertical_gap[j] + gap_open[j];
      last_vertical_gap[j] += gap_extension[j];
    }
  }

  for (size_t row = 1; row <= sequences1_t.size();
      ++row) {

    const auto& sequence1 = sequences1_t[row - 1];

    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.
    score_column[0] = horizontal_gap_column[0];
    for (size_t j = 0; j < sequence_count_v; ++j) {
      last_vertical_gap[j] = horizontal_gap_column[0][j] + gap_open[j];
      horizontal_gap_column[0][j] += gap_extension[j];
    }

    /*
    * Compute the main recursion to fill the matrix.
    */
    for (unsigned col = 1; col <= sequences2_t.size(); ++col) {
      const auto& sequence2 = sequences2_t[col - 1];
      // Compute next score from diagonal direction with match/mismatch.

      score_t best_cell_score = last_diagonal_score;
      for (size_t j = 0; j < sequence_count_v; ++j) {
        best_cell_score[j] += (sequence1[j] == sequence2[j] ? match[j] : mismatch[j]);
        best_cell_score[j] = std::max(best_cell_score[j], last_vertical_gap[j]);
        best_cell_score[j] = std::max(best_cell_score[j], horizontal_gap_column[col][j]);
      }

      // Cache next diagonal value and store optimum in score_column.
      last_diagonal_score = score_column[col];
      score_column[col] = best_cell_score;
      // Compute the next values for vertical and horizontal gap.

      for (size_t j = 0; j < sequence_count_v; ++j) {
        best_cell_score[j] += gap_open[j];
        last_vertical_gap[j] += gap_extension[j];
        horizontal_gap_column[col][j] += gap_extension[j];
        last_vertical_gap[j] = std::max(last_vertical_gap[j], best_cell_score[j]);
        horizontal_gap_column[col][j] =
            std::max(horizontal_gap_column[col][j], best_cell_score[j]);
      }
    }

    // Report the best score.
    result = score_column.back();
  }

  return result;
}
