#include "solution.hpp"
#include <cassert>
#include <type_traits>

using transposedSequences = std::array<result_t, sequence_size_v>;

static transposedSequences transposeSequences(std::vector<sequence_t> const &sequences) {
  transposedSequences transposed;

  for (size_t i = 0; i < sequence_count_v; ++i) {
    for (size_t j = 0; j < sequence_size_v; ++j) {
      transposed[j][i] = sequences[i][j];
    }
  }

  return transposed;
}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1, std::vector<sequence_t> const &sequences2) {
  result_t result{};

  const transposedSequences &sequences1Transposed = transposeSequences(sequences1);
  const transposedSequences &sequences2Transposed = transposeSequences(sequences2);

  using score_t = int16_t;
  using score_arr_t = std::array<int16_t, sequence_count_v>;
  using column_arr_t = std::array<score_arr_t, sequence_size_v + 1>;

  /*
   * Initialise score values.
   */
  const score_t gap_open{ -11 };
  const score_t gap_extension{ -1 };
  const score_t match{ 6 };
  const score_t mismatch{ -4 };

  /*
   * Setup the matrix.
   * Note we can compute the entire matrix with just one column in memory,
   * since we are only interested in the last value of the last column in the
   * score matrix.
   */
  column_arr_t score_column{};
  column_arr_t horizontal_gap_column{};
  score_arr_t last_vertical_gap{};

  /*
   * Initialise the first column of the matrix.
   */
  for (size_t s = 0; s < sequence_count_v; ++s) {
    horizontal_gap_column[0][s] = gap_open;
    last_vertical_gap[s] = gap_open;
  }

  for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t s = 0; s < sequence_count_v; ++s) {
      score_column[i][s] = last_vertical_gap[s];
      horizontal_gap_column[i][s] = last_vertical_gap[s] + gap_open;
      last_vertical_gap[s] += gap_extension;
    }
  }

  // Cache arrays
  column_arr_t last_vertical_gap_arr;
  column_arr_t last_score_column;

  /*
   * Compute the main recursion to fill the matrix.
   */
  for (size_t col = 1; col <= sequences2Transposed.size(); ++col) {
    last_score_column = score_column;

    for (size_t s = 0; s < sequence_count_v; ++s) {
      score_column[0][s] = horizontal_gap_column[0][s];
      last_vertical_gap[s] = score_column[0][s] + gap_open;
      horizontal_gap_column[0][s] += gap_extension;
    }

    for (size_t row = 1; row <= sequences1Transposed.size(); ++row) {
      for (size_t s = 0; s < sequence_count_v; ++s) {
        // Compute next score from diagonal direction with match/mismatch.
        score_t best_cell_score = last_score_column[row - 1][s] + (sequences1Transposed[row - 1][s] == sequences2Transposed[col - 1][s] ? match : mismatch); // ok

        // Determine best score from diagonal and horizontal direction.
        best_cell_score = max(best_cell_score, horizontal_gap_column[row][s]); // ok

        // Store optimum in score_column.
        score_column[row][s] = best_cell_score;

        // Store optimum between gap open and gap extension.
        last_vertical_gap_arr[row - 1][s] = last_vertical_gap[s];
        last_vertical_gap[s] = max(last_vertical_gap[s] + gap_extension, best_cell_score + gap_open);
      }
    }

    for (size_t row = 1; row <= sequences1Transposed.size(); ++row) {
      for (size_t s = 0; s < sequence_count_v; ++s) {
        score_column[row][s] = max(score_column[row][s], last_vertical_gap_arr[row - 1][s]);
        horizontal_gap_column[row][s] = max(horizontal_gap_column[row][s] + gap_extension, score_column[row][s] + gap_open);
      }
    }
  }


  for (size_t s = 0; s < sequence_count_v; ++s) {
    // Report the best score.
    result[s] = score_column.back()[s];
  }

  return result;
}
