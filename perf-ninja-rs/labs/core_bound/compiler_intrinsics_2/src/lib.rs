use std::cmp::max;

#[cfg(test)]
mod tests;

// Find the longest line in a file.
// Implementation uses ternary operator with a hope that compiler will
// turn it into a CMOV instruction.
// The code inside the inner loop is equivalent to:
/*
if (s == '\n') {
  longestLine = std::max(curLineLength, longestLine);
  curLineLength = 0;
} else {
  curLineLength++;
}*/
pub fn solution(input_contents: &str) -> u32 {
    let mut longest_line = 0;
    let mut cur_line_length = 0;

    for s in input_contents.chars() {
        cur_line_length = if s == '\n' { 0 } else { cur_line_length + 1 };
        longest_line = max(cur_line_length, longest_line);
    }

    longest_line
}
