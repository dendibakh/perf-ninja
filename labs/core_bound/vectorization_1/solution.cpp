#include "solution.hpp"
#include <algorithm>
#include <cassert>

using simd_sequences_t = std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v>;

simd_sequences_t transpose(std::vector<sequence_t> const &sequences) {
  simd_sequences_t sequences_tr{};
  for (int row = 0; row < sequences.size(); ++row) {
    for (int col = 0; col < sequences[row].size(); ++col) {
      sequences_tr[col][row] = sequences[row][col];
    }
  }
  return sequences_tr;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  result_t result{};

  simd_sequences_t sequences1_tr = transpose(sequences1);
  simd_sequences_t sequences2_tr = transpose(sequences2);

  // pprint(sequences1);
  // pprint(sequences1_tr);

  using score_t = int16_t;
  using simd_score_t = std::array<score_t, sequence_count_v>;
  using column_t = std::array<simd_score_t, sequence_size_v + 1>;

  /*
   * Initialise score values.
   */
  score_t gap_open{-11};
  score_t gap_extension{-1};
  score_t match{6};
  score_t mismatch{-4};

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  column_t score_column{};
  column_t horizontal_gap_column{};
  simd_score_t last_vertical_gap{};

  /*
   * Initialise the first column of the matrix.
   */
  for (int k = 0; k < sequence_count_v; ++k) {
    horizontal_gap_column[0][k] = gap_open;
    last_vertical_gap[k] = gap_open;
  }

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (int k = 0; k < sequence_count_v; ++k) {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open;
      last_vertical_gap[k] += gap_extension;
    }
  }

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (unsigned col = 1; col <= sequences2_tr.size(); ++col) {
    simd_score_t last_diagonal_score = score_column[0]; // Cache last diagonal score to compute this cell.
    for (int k = 0; k < sequence_count_v; ++k) {
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open;
      horizontal_gap_column[0][k] += gap_extension;
    }

    for (unsigned row = 1; row <= sequences1_tr.size(); ++row) {
      simd_score_t best_cell_score = last_diagonal_score;

      for (int k = 0; k < sequence_count_v; ++k)
        best_cell_score[k] += (sequences1_tr[row - 1][k] == sequences2_tr[col - 1][k] ? match : mismatch);

      for (int k = 0; k < sequence_count_v; ++k) {
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[row][k]);
        // Cache next diagonal value and store optimum in score_column.
        last_diagonal_score[k] = score_column[row][k];
        // scores[row] = score_column[row];
        score_column[row][k] = best_cell_score[k];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[k] += gap_open;
        last_vertical_gap[k] += gap_extension;
        horizontal_gap_column[row][k] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[row][k] =
            std::max(horizontal_gap_column[row][k], best_cell_score[k]);
      }
    }
  }

  // Report the best score.
  for (int k = 0; k < sequence_count_v; ++k) {
    result[k] = score_column.back()[k];
  }

  return result;
}
