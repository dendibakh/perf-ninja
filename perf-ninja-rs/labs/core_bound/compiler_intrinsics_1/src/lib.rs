use std::cmp::min;

#[cfg(test)]
mod tests;

pub type InputVector = Vec<u8>;
pub type OutputVector = Vec<u16>;
pub const RADIUS: usize = 13; // assume diameter (2 * radius + 1) to be less
                              // than 256 so results fits in uint16_t

pub fn image_smoothing(input: &InputVector, radius: usize, output: &mut OutputVector) {
    let mut pos = 0;
    let mut current_sum: u16 = 0;
    let size = input.len();

    // 1. left border - time spend in this loop can be ignored, no need to
    // optimize it
    for i in 0..min(size, radius) {
        current_sum += input[i] as u16;
    }

    let mut limit = min(radius + 1, size.saturating_sub(radius));
    while pos < limit {
        current_sum += input[pos + radius] as u16;
        output[pos] = current_sum;
        pos += 1;
    }

    // 2. main loop.
    limit = size.saturating_sub(radius);
    while pos < limit {
        current_sum -= input[pos - radius - 1] as u16;
        current_sum += input[pos + radius] as u16;
        output[pos] = current_sum;
        pos += 1;
    }

    // 3. special case, executed only if size <= 2*radius + 1
    limit = min(radius + 1, size);
    while pos < limit {
        output[pos] = current_sum;
        pos += 1;
    }

    // 4. right border - time spend in this loop can be ignored, no need to
    // optimize it
    while pos < size {
        current_sum -= input[pos - radius - 1] as u16;
        output[pos] = current_sum;
        pos += 1;
    }
}

const N: usize = 40_000;

pub fn init() -> InputVector {
    use rand::prelude::*;
    let mut generator = thread_rng();
    let mut data = Vec::with_capacity(N);
    data.resize_with(N, || generator.gen::<u8>());
    data
}
