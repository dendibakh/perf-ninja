use crate::{init, solution, N, S};
use libc::{c_int, c_void, qsort, size_t};
use std::cmp::Ordering;
use std::mem::size_of;

#[test]
fn validate() {
    let mut arr = init();

    let mut expected = arr; // copy
    solution(&mut arr);
    original_solution(&mut expected);

    assert_eq!(arr, expected);
}

fn original_compare(a: &S, b: &S) -> Ordering {
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
    original_compare(&*(lhs as *const S), &*(rhs as *const S)) as c_int
}

fn original_solution(arr: &mut [S; N]) {
    unsafe {
        qsort(
            arr.as_mut_ptr() as *mut c_void,
            N as size_t,
            size_of::<S>() as size_t,
            Some(qsort_compare),
        );
    }
}
