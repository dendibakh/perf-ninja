use std::cmp::max;

#[cfg(test)]
mod tests;

pub const SEQUENCE_SIZE: usize = 200; // The length of the generated sequences.
pub const SEQUENCE_COUNT: usize = 16; // The number of sequences to generate for both sequence collections.

pub type Sequence = [u8; SEQUENCE_SIZE];
pub type AlignResult = [i16; SEQUENCE_COUNT];

// The alignment algorithm which computes the alignment of the given sequence
// pairs.
pub fn compute_alignment(sequences1: &[Sequence], sequences2: &[Sequence]) -> AlignResult {
    let mut result: AlignResult = [0; SEQUENCE_COUNT];

    for sequence_idx in 0..sequences1.len() {
        type Score = i16;
        type Column = [Score; SEQUENCE_SIZE + 1];

        let sequence1: &Sequence = &sequences1[sequence_idx];
        let sequence2: &Sequence = &sequences2[sequence_idx];

        /*
         * Initialise score values.
         */
        let gap_open: Score = -11;
        let gap_extension: Score = -1;
        let match_score: Score = 6; // previously 'match'
        let mismatch: Score = -4;

        /*
         * Setup the matrix.
         * Note we can compute the entire matrix with just one column in memory,
         * since we are only interested in the last value of the last column in the
         * score matrix.
         */
        let mut score_column: Column = [0; SEQUENCE_SIZE + 1];
        let mut horizontal_gap_column: Column = [0; SEQUENCE_SIZE + 1];
        let mut last_vertical_gap: Score;

        /*
         * Initialise the first column of the matrix.
         */
        horizontal_gap_column[0] = gap_open;
        last_vertical_gap = gap_open;

        for i in 1..score_column.len() {
            score_column[i] = last_vertical_gap;
            horizontal_gap_column[i] = last_vertical_gap + gap_open;
            last_vertical_gap += gap_extension;
        }

        /*
         * Compute the main recursion to fill the matrix.
         */
        for col in 1..=sequence2.len() {
            let mut last_diagonal_score: Score = score_column[0]; // Cache last diagonal score to compute this cell.
            score_column[0] = horizontal_gap_column[0];
            last_vertical_gap = horizontal_gap_column[0] + gap_open;
            horizontal_gap_column[0] += gap_extension;

            let diag = sequence2[col - 1];
            for row in 1..=sequence1.len() {
                // Compute next score from diagonal direction with match/mismatch.
                let mut best_cell_score = last_diagonal_score
                    + (if sequence1[row - 1] == diag {
                        match_score
                    } else {
                        mismatch
                    });
                // Determine best score from diagonal, vertical, or horizontal
                // direction.
                best_cell_score = max(best_cell_score, last_vertical_gap);
                best_cell_score = max(best_cell_score, horizontal_gap_column[row]);
                // Cache next diagonal value and store optimum in score_column.
                last_diagonal_score = score_column[row];
                score_column[row] = best_cell_score;
                // Compute the next values for vertical and horizontal gap.
                best_cell_score += gap_open;
                last_vertical_gap += gap_extension;
                horizontal_gap_column[row] += gap_extension;
                // Store optimum between gap open and gap extension.
                last_vertical_gap = max(last_vertical_gap, best_cell_score);
                horizontal_gap_column[row] = max(horizontal_gap_column[row], best_cell_score);
            }
        }

        // Report the best score.
        result[sequence_idx] = *score_column.last().unwrap();
    }

    result
}

// C++ version includes a branchless max which is ported here (copied from crate lokacore) but
// not used as it doesn't seem to affect branch-misses, and is slower.
//
// Clang-12 compiler generates branches for std::max, which are often mispredicted
// in this benchmark. That's the reason we provide branchless version of max function.
//fn max(a: i16, b: i16) -> i16 {
//    a ^ ((a ^ b) & ((a < b) as i16).wrapping_neg())
//}

// Initialises a pair of sequence collections given a fixed sequence size.
pub fn init() -> (Vec<Sequence>, Vec<Sequence>) {
    use rand::distributions::Uniform;
    use rand::prelude::*;
    let mut random_engine = thread_rng();

    // Simulate DNA alphabet with 4 symbols.
    let symbol_distribution = Uniform::from(0..4);

    let mut generate_sequences = || -> Vec<Sequence> {
        let mut sequences: Vec<Sequence> = vec![[0u8; SEQUENCE_SIZE]; SEQUENCE_COUNT];
        for sequence in &mut sequences {
            sequence.fill_with(|| symbol_distribution.sample(&mut random_engine));
        }
        sequences
    };

    (generate_sequences(), generate_sequences())
}
