#include "solution.hpp"
#include <algorithm> 
#include <cassert> 
#include <type_traits> 


using sequence_t = std::array<uint8_t, sequence_size_v>; 
using transpose_t = std::array<std::array<uint16_t, sequence_count_v>, sequence_size_v>;

void transpose(transpose_t &transp_seq, std::vector<sequence_t> const &sequence) {
  for (size_t i = 0; i < sequence_size_v; ++i) {
    for (size_t j = 0; j < sequence_count_v; ++j) {
      transp_seq[i][j] = sequence[j][i];
    }
  }
}

result_t compute_alignment(std::vector<sequence_t> const &sequence1,
                           std::vector<sequence_t> const &sequence2) {

  result_t result{};

  transpose_t transpose_seq1{};
  transpose_t transpose_seq2{};

  transpose(transpose_seq1, sequence1);
  transpose(transpose_seq2, sequence2);

  using score_t = std::array<int16_t, sequence_count_v>;
  using column_t = std::array<score_t, sequence_size_v + 1>;


  score_t gap_open{};
  gap_open.fill(-11);

  score_t gap_extension{};
  gap_extension.fill(-1);

  score_t match{};
  match.fill(6);

  score_t mismatch{};
  mismatch.fill(-4);


  column_t score_column{};
  column_t horizontal_gap_column{};
  score_t last_vertical_gap{};


  horizontal_gap_column[0] = gap_open;
  last_vertical_gap = gap_open;
  
  for (size_t i = 1; i < score_column.size(); ++i) {
    for (size_t k = 0; k < sequence_count_v; ++k) {
      score_column[i][k] = last_vertical_gap[k];
      horizontal_gap_column[i][k] = last_vertical_gap[k] + gap_open[k];
      last_vertical_gap[k] += gap_extension[k];
    }
  }

  for (unsigned idx_out = 1; idx_out <= transpose_seq2.size(); ++idx_out) {
    score_t last_diagonal_score = score_column[0];

    for (size_t k = 0; k < sequence_count_v; ++k) {
      score_column[0][k] = horizontal_gap_column[0][k];
      last_vertical_gap[k] = horizontal_gap_column[0][k] + gap_open[k];
      horizontal_gap_column[0][k] += gap_extension[k];
    }
  
    for (unsigned idx_in = 1; idx_in <= transpose_seq1.size(); ++idx_in) {
      score_t best_cell_score{};

      for (size_t k = 0; k < sequence_count_v; ++k) { 
        best_cell_score[k] = 
          last_diagonal_score[k] + 
          (transpose_seq1[idx_in - 1][k] == transpose_seq2[idx_out - 1][k] ? match[k] : mismatch[k]);
      }

      for (size_t k = 0; k < sequence_count_v; ++k) { 
        best_cell_score[k] = std::max(best_cell_score[k], last_vertical_gap[k]);
        best_cell_score[k] = std::max(best_cell_score[k], horizontal_gap_column[idx_in][k]);

        last_diagonal_score[k] = score_column[idx_in][k];
        score_column[idx_in][k] = best_cell_score[k];

        best_cell_score[k] += gap_open[k];
        last_vertical_gap[k] += gap_extension[k];
        horizontal_gap_column[idx_in][k] += gap_extension[k];
        // Store optimum between gap open and gap extension.
        last_vertical_gap[k] = std::max(last_vertical_gap[k], best_cell_score[k]);
        horizontal_gap_column[idx_in][k] =
            std::max(horizontal_gap_column[idx_in][k], best_cell_score[k]);
      }
    }
  }

  for (size_t k = 0; k < sequence_count_v; ++k) { 
    result[k] = score_column.back()[k];
  }

  return result;
}
