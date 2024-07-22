#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <iostream>

typedef int16_t vec16 __attribute__((vector_size(16 * sizeof(int16_t))));

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  // transpose 2 sequences
  // from N * L to L * N
  std::array<vec16, sequence_size_v> trans_seq1s;
  for (int i = 0; i < sequence_count_v; i++) {
    for (int j = 0; j < sequence_size_v; j++) {
      trans_seq1s[j][i] = sequences1[i][j];
    }
  }
  std::array<vec16, sequence_size_v> trans_seq2s;
  for (int i = 0; i < sequence_count_v; i++) {
    for (int j = 0; j < sequence_size_v; j++) {
      trans_seq2s[j][i] = sequences2[i][j];
    }
  }

  result_t result{};

  using score_t = vec16;
  using column_t = std::array<score_t, sequence_size_v + 1>;

  /*
    * Initialise score values.
    */
  score_t gap_open = score_t{} + -11;
  score_t gap_extension = score_t{} + -1;
  score_t match = score_t{} + 6;
  score_t mismatch = score_t{} + -4;

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
    score_column[i] = last_vertical_gap;
    horizontal_gap_column[i] = last_vertical_gap + gap_open;
    last_vertical_gap += gap_extension;
  }

  auto get_max = [](vec16 a, vec16 b) {
    return ((a <= b) & b) + ((a > b) & a);
  };

  auto debug = [](vec16 a) {
    for (int i = 0; i < 16; i++) {
      std::cout << a[i] << " ";
    }
    std::cout << "\n";
  };

  /*
    * Compute the main recursion to fill the matrix.
    */
  for (unsigned col = 1; col <= sequence_size_v; ++col) {
    score_t last_diagonal_score =
        score_column[0]; // Cache last diagonal score to compute this cell.
    score_column[0] = horizontal_gap_column[0];
    last_vertical_gap = horizontal_gap_column[0] + gap_open;
    horizontal_gap_column[0] += gap_extension;

    for (unsigned row = 1; row <= sequence_size_v; ++row) {
      // Compute next score from diagonal direction with match/mismatch.
      score_t diff = (trans_seq1s[row - 1] == trans_seq2s[col - 1]) & match;
      diff += (trans_seq1s[row - 1] != trans_seq2s[col - 1]) & mismatch; 
      score_t best_cell_score = last_diagonal_score + diff;
      // Determine best score from diagonal, vertical, or horizontal
      // direction.
      best_cell_score = get_max(best_cell_score, last_vertical_gap);
      best_cell_score = get_max(best_cell_score, horizontal_gap_column[row]);
      // Cache next diagonal value and store optimum in score_column.
      last_diagonal_score = score_column[row];
      score_column[row] = best_cell_score;
      // Compute the next values for vertical and horizontal gap.
      best_cell_score += gap_open;
      last_vertical_gap += gap_extension;
      horizontal_gap_column[row] += gap_extension;
      // Store optimum between gap open and gap extension.
      last_vertical_gap = get_max(last_vertical_gap, best_cell_score);
      horizontal_gap_column[row] =
          get_max(horizontal_gap_column[row], best_cell_score);
    }
  }

    // Report the best score.
    // result[sequence_idx] = score_column.back();
    for (int i = 0; i < sequence_count_v; i++) {
      result[i] = score_column.back()[i];
    }

  return result;
}
