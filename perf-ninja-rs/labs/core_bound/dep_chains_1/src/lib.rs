#![feature(allocator_api)]

#[cfg(test)]
mod tests;

mod init;
pub use init::get_random_list;

mod arena;
pub use arena::Arena;

const N: usize = 10_000;

pub struct List {
    pub next: *const List,
    pub value: u32,
}

pub fn get_sum_of_digits(mut n: u32) -> u32 {
    let mut sum = 0u32;
    while n != 0 {
        sum += n % 10;
        n /= 10;
    }
    sum
}

// Task: lookup all the values from l2 in l1.
// For every found value, find the sum of its digits.
// Return the sum of all digits in every found number.
// Both lists have no duplicates and elements placed in *random* order.
// Do NOT sort any of the lists. Do NOT store elements in a hash_map/sets.

// Hint: Traversing a linked list is a long data dependency chain:
//       to get the node N+1 you need to retrieve the node N first.
//       Think how you can execute multiple dependency chains in parallel.
//
/// # Safety
/// Basic linked-list iteration, what could possibly go wrong?
pub unsafe fn solution(mut l1: *const List, mut l2: *const List) -> u32 {
    let mut ret_val = 0u32;

    let head2 = l2;
    // O(N^2) algorithm:
    while !l1.is_null() {
        let v = (*l1).value;
        l2 = head2;
        while !l2.is_null() {
            if (*l2).value == v {
                ret_val += get_sum_of_digits(v);
                break;
            }
            l2 = (*l2).next;
        }
        l1 = (*l1).next;
    }

    ret_val
}
