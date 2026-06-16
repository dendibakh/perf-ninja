#![feature(maybe_uninit_array_assume_init)]

use std::cmp::Ordering;
use std::mem::{size_of, MaybeUninit};

use libc::{c_int, c_void, qsort, size_t};

#[cfg(test)]
mod tests;

// Assume this constant never changes
pub const N: usize = 10_000;

#[derive(Debug, PartialEq, Eq, Copy, Clone)]
pub struct S {
    pub key1: u32,
    pub key2: u32,
}

fn compare(a: &S, b: &S) -> Ordering {
    if a.key1 < b.key1 {
        return Ordering::Less;
    }

    if a.key1 > b.key1 {
        return Ordering::Greater;
    }

    if a.key2 < b.key2 {
        return Ordering::Less;
    }

    if a.key2 > b.key2 {
        return Ordering::Greater;
    }

    Ordering::Equal
}

unsafe extern "C" fn qsort_compare(lhs: *const c_void, rhs: *const c_void) -> c_int {
    compare(&*(lhs as *const S), &*(rhs as *const S)) as c_int
}

pub fn solution(arr: &mut [S; N]) {
    unsafe {
        qsort(
            arr.as_mut_ptr() as *mut c_void,
            N as size_t,
            size_of::<S>() as size_t,
            Some(qsort_compare),
        );
    }
}

pub fn init() -> [S; N] {
    use rand::distributions::Uniform;
    use rand::prelude::*;

    let mut arr: [MaybeUninit<S>; N] = std::array::from_fn(|_| MaybeUninit::uninit());
    let mut generator = thread_rng();
    let distribution = Uniform::from(0..9000);
    for i in 0..N {
        arr[i].write(S {
            key1: distribution.sample(&mut generator),
            key2: distribution.sample(&mut generator),
        });
    }
    unsafe { MaybeUninit::array_assume_init(arr) }
}
