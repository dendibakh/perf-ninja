#![feature(maybe_uninit_array_assume_init)]

use rand::prelude::*;

use std::cmp::{Ord, Ordering, PartialOrd};

#[cfg(test)]
mod tests;

mod init;
pub use init::{create_entry, init};

// Assume those constants never change
pub const N: usize = 10_000;
pub const MIN_RANDOM: i32 = 0;
pub const MAX_RANDOM: i32 = 100;

// FIXME: this data structure can be reduced in size
#[repr(C)]
#[derive(Debug, Copy, Clone, Default)]
pub struct S {
    pub i: i32,
    pub l: i64,
    pub s: i16,
    pub d: f64,
    pub b: bool,
}

// C++ version overloads '<' operator like this:
//   bool operator<(const S &s) const { return this->i < s.i; }
// I presume that means we order / sort / compare based only on value of 'i'?

impl PartialOrd for S {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.i.partial_cmp(&other.i)
    }
}
impl Ord for S {
    fn cmp(&self, other: &Self) -> Ordering {
        self.i.cmp(&other.i)
    }
}
impl PartialEq for S {
    fn eq(&self, other: &Self) -> bool {
        self.i == other.i
    }
}
impl Eq for S {}

pub fn solution(arr: &mut [S]) {
    // 1. shuffle
    let mut rd = thread_rng();
    arr.shuffle(&mut rd);

    // 2. sort
    arr.sort_unstable();
}
