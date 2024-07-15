#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>

std::array<result_t, sequence_size_v> transpose(std::vector<sequence_t> const &sequences) {
  assert(sequences.size() == sequence_count_v);
  assert(sequences[0].size() == sequence_size_v);

  std::array<result_t, sequence_size_v> result{};
  for (int i = 0; i < sequence_size_v; ++i)
    for (int j = 0; j < sequence_count_v; ++j)
      result[i][j] = sequences[j][i];

  return result;

}

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
result_t compute_alignment(std::vector<sequence_t> const &sequences1,
                           std::vector<sequence_t> const &sequences2) {
  auto seq1T = transpose(sequences1);
  auto seq2T = transpose(sequences2);

  using score_t = int16_t;
  using column_t = std::array<score_t, sequence_size_v + 1>;
  
  result_t result{};


  score_t gap_open{-11};
  score_t gap_extension{-1};
  score_t match{6};
  score_t mismatch{-4};

  std::array<column_t, sequence_count_v> all_score_column{};
  std::array<column_t, sequence_count_v> all_horizontal_gap_column{};
  std::array<score_t, sequence_count_v> all_last_vertical_gap{};
  std::array<score_t, sequence_count_v> all_last_diagonal_score{};


  for (int idx = 0; idx < sequence_count_v; ++idx) {
    all_horizontal_gap_column[idx][0] = gap_open;
    all_last_vertical_gap[idx] = gap_open;
    all_score_column[idx][0] = 0;

    for (int j = 1; j < sequence_size_v + 1; ++j) {
      all_score_column[idx][j] = all_last_vertical_gap[idx];
      all_horizontal_gap_column[idx][j] = all_last_vertical_gap[idx] + gap_open;
      all_last_vertical_gap[idx] += gap_extension;
    }
  }
  
  for (int seq2_col = 0; seq2_col < sequence_size_v; ++seq2_col) {
    for (int idx = 0; idx < sequence_count_v; ++idx) {
      all_last_diagonal_score[idx] = all_score_column[idx][0];
      all_score_column[idx][0] = all_horizontal_gap_column[idx][0];
      all_last_vertical_gap[idx] = all_horizontal_gap_column[idx][0] + gap_open;
      all_horizontal_gap_column[idx][0] += gap_extension;
    }

    for (int seq1_col = 0; seq1_col < sequence_size_v; ++seq1_col) {

      for (int idx = 0; idx < sequence_count_v; ++idx) {
        score_t best_cell_score = all_last_diagonal_score[idx] +
          (seq1T[seq1_col][idx] == seq2T[seq2_col][idx] ? match : mismatch);
        
        best_cell_score = std::max(best_cell_score, all_last_vertical_gap[idx]);
        best_cell_score = std::max(best_cell_score, all_horizontal_gap_column[idx][seq1_col+1]);
      
        all_last_diagonal_score[idx] = all_score_column[idx][seq1_col+1];
        all_score_column[idx][seq1_col+1] = best_cell_score;

        best_cell_score += gap_open;
        all_last_vertical_gap[idx] += gap_extension;
        all_horizontal_gap_column[idx][seq1_col+1] += gap_extension;

        all_last_vertical_gap[idx] = std::max(all_last_vertical_gap[idx], best_cell_score);
        all_horizontal_gap_column[idx][seq1_col+1] = std::max(all_horizontal_gap_column[idx][seq1_col+1], best_cell_score);
      }

    }

  }

  for (int idx = 0; idx < sequence_count_v; ++idx)
    result[idx] = all_score_column[idx].back();

  return result;
}
