#![cfg(test)]

use crate::{checksum, init, Blob, N};
use std::num::Wrapping;

fn original_checksum(blob: &Blob) -> u16 {
    let mut acc = Wrapping(0);
    for value in blob {
        acc += value;
        acc += (acc.0 < *value) as u16; // add carry
    }
    acc.0
}

#[test]
fn validate() {
    let mut blob: Blob = [0; N];
    init(&mut blob);

    let original_result = original_checksum(&blob);
    let result = checksum(&blob);

    assert_eq!(original_result, result);
}
