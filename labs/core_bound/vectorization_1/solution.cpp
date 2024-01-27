#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

using trasposed_seq = std::array<std::array<uint8_t, sequence_count_v>, sequence_size_v>;

trasposed_seq transpose(std::vector<sequence_t> const &sequences) {
  trasposed_seq ts{};
  for (size_t i = 0; i < sequences.size(); ++i) {
    for (size_t j = 0; j < sequences[0].size(); ++j) {
      ts[j][i] = sequences[i][j];
    }
  }
  return ts;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  auto ts1 = transpose(sequences1);
  auto ts2 = transpose(sequences2);
  result_t result{};

  using score_t = int16_t;
  using scores_t = std::array<int16_t, sequence_count_v>;
  using column_t = std::array<scores_t, sequence_size_v + 1>;

  /*
    * Initialise score values.
    */
  const score_t gap_open{-11};
  const score_t gap_extension{-1};
  const score_t match{6};
  const score_t mismatch{-4};

  /*
    * Setup the matrix.
    * Note we can compute the entire matrix with just one column in memory,
    * since we are only interested in the last value of the last column in the
    * score matrix.
    */
  column_t score_column{};
  column_t horizontal_gap_column{};
  scores_t last_vertical_gap{};
  std::array<scores_t, std::tuple_size<sequence_t>::value> mathces;
  /*
    * Initialise the first column of the matrix.
    */
  for (size_t seq_num = 0; seq_num < sequence_count_v; ++seq_num) {
    horizontal_gap_column[0][seq_num] = gap_open;
    last_vertical_gap[seq_num] = gap_open;
  }

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t seq_num = 0; seq_num < sequence_count_v; ++seq_num) {
      score_column[i][seq_num] = last_vertical_gap[seq_num];
      horizontal_gap_column[i][seq_num] = last_vertical_gap[seq_num] + gap_open;
      last_vertical_gap[seq_num] += gap_extension;
    }
  }

  /*
    * Compute the main recursion to fill the matrix.
    */
  for (unsigned col = 1; col <= ts2.size(); ++col) {
    for (size_t seq_num = 0; seq_num < sequence_count_v; ++seq_num) {
      for (unsigned row = 1; row <= ts1.size(); ++row) {
        mathces[row-1][seq_num] = score_column[row-1][seq_num] + (ts1[row - 1][seq_num] == ts2[col - 1][seq_num] ? match : mismatch);
      }
    }
    for (size_t seq_num = 0; seq_num < sequence_count_v; ++seq_num) {
      score_column[0][seq_num] = horizontal_gap_column[0][seq_num];
      last_vertical_gap[seq_num] = horizontal_gap_column[0][seq_num] + gap_open;
      horizontal_gap_column[0][seq_num] += gap_extension;
    }

    for (unsigned row = 1; row <= ts1.size(); ++row) {
      // Compute next score from diagonal direction with match/mismatch.
      scores_t best_cell_score{};
      for (size_t seq_num = 0; seq_num < sequence_count_v; ++seq_num) {
        best_cell_score[seq_num] = mathces[row - 1][seq_num];
        // Determine best score from diagonal, vertical, or horizontal
        // direction.
        best_cell_score[seq_num] = std::max(best_cell_score[seq_num], last_vertical_gap[seq_num]);
        best_cell_score[seq_num] = std::max(best_cell_score[seq_num], horizontal_gap_column[row][seq_num]);
        // Cache next diagonal value and store optimum in score_column.
        score_column[row][seq_num] = best_cell_score[seq_num];
        // Compute the next values for vertical and horizontal gap.
        best_cell_score[seq_num] += gap_open;
        last_vertical_gap[seq_num] += gap_extension;
        horizontal_gap_column[row][seq_num] += gap_extension;
        // Store optimum between gap open and gap extension.
        last_vertical_gap[seq_num] = std::max(last_vertical_gap[seq_num], best_cell_score[seq_num]);
        horizontal_gap_column[row][seq_num] =
            std::max(horizontal_gap_column[row][seq_num], best_cell_score[seq_num]);
      }
    }
  }

  // Report the best score.
  for (size_t seq_num = 0; seq_num < sequence_count_v; ++seq_num) {
    result[seq_num] = score_column.back()[seq_num];
  }

  return result;
}
