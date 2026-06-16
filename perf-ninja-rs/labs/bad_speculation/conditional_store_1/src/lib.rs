use rand::prelude::*;

// Assume this constant never changes
pub const N: usize = 64 * 1024;

// "first" contains a metric
// "second" contains associated data
pub type S = (u32, u32);

// Select items which have S.first in range [lower..upper]
pub fn select(output: &mut [S; N], input: &[S; N], lower: u32, upper: u32) -> usize {
    let mut count = 0;
    for item in input {
        if (lower <= item.0) && (item.0 <= upper) {
            output[count] = *item;
            count += 1;
        }
    }
    count
}

pub fn init() -> [S; N] {
    let mut generator = thread_rng();

    let mut arr = [(0, 0); N];
    for i in 0..N {
        arr[i].0 = generator.gen();
        arr[i].1 = generator.gen();
    }

    arr
}
