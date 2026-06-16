use crate::{create_entry, init, solution, MAX_RANDOM, MIN_RANDOM, N};

#[test]
fn validate() {
    let mut arr = init();

    let mut expected = arr.clone(); // copy
    solution(&mut arr);
    expected.sort_unstable();

    for i in 0..N {
        // we only check i components since sort can be unstable
        assert_eq!(arr[i].i, expected[i].i);
    }

    let mut checks_passed = check_entry(MIN_RANDOM, MIN_RANDOM);
    checks_passed = check_entry(MIN_RANDOM, MAX_RANDOM) && checks_passed;
    checks_passed = check_entry(MIN_RANDOM + 1, MAX_RANDOM - 1) && checks_passed;
    checks_passed = check_entry(MAX_RANDOM, MIN_RANDOM) && checks_passed;
    checks_passed = check_entry(MAX_RANDOM, MAX_RANDOM) && checks_passed;

    assert!(checks_passed);
}

fn check_entry(first: i32, second: i32) -> bool {
    let entry = create_entry(first, second);

    let mut is_valid = true;

    if entry.i != first {
        report_error("i", entry.i, first, first, second);
        is_valid = false;
    }

    if entry.s != second as i16 {
        report_error("s", entry.s, second, first, second);
        is_valid = false;
    }

    let expected_l = (first * second) as i64;
    if entry.l != expected_l {
        report_error("l", entry.l, expected_l, first, second);
        is_valid = false;
    }

    let expected_d = first as f64 / MAX_RANDOM as f64;
    if (entry.d - expected_d).abs() > 0.001 {
        report_error("d", entry.d, expected_d, first, second);
        is_valid = false;
    }

    let expected_b = first < second;
    if entry.b != expected_b {
        report_error("b", entry.b, expected_b, first, second);
        is_valid = false;
    }

    is_valid
}

fn report_error<T: std::fmt::Debug, E: std::fmt::Debug>(
    var_name: &str,
    received: T,
    expected: E,
    first_value: i32,
    second_value: i32,
) {
    eprintln!(
        "Validation Failed. Value {var_name} is {received:?}
            . Expected is {expected:?} for intialization values
            {first_value} and {second_value}"
    );
}
