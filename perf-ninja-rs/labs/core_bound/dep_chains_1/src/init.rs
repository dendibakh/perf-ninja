use rand::distributions::Uniform;
use rand::prelude::*;
use std::ptr::null;

use crate::Arena;
use crate::{List, N};

pub fn get_random_vector() -> Vec<u32> {
    let mut generator = thread_rng();
    let distribution = Uniform::from(0..u16::MAX);

    let mut ret_vector: Vec<u32> = Vec::with_capacity(N);
    ret_vector.resize_with(N, || distribution.sample(&mut generator).into());

    // remove duplicates
    ret_vector.sort();
    ret_vector.dedup();

    // reshuffle
    ret_vector.shuffle(&mut generator);
    ret_vector
}

pub fn get_random_list(allocator: &Arena) -> *const List {
    let create_node = |v: u32| -> *mut List {
        let b = Box::new_in(
            List {
                value: v,
                next: null(),
            },
            allocator,
        );
        Box::into_raw_with_allocator(b).0
    };

    let head = create_node(0);
    let mut l = head;
    let randoms = get_random_vector();
    for v in randoms {
        unsafe {
            (*l).next = create_node(v);
            l = (*l).next as *mut List;
        }
    }

    head
}

// For debugging
fn _print_list(mut l: *const List) {
    while !l.is_null() {
        unsafe {
            println!("{} ", (*l).value);
            l = (*l).next;
        }
    }
    println!();
}
