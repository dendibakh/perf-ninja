use crate::{get_random_list, get_sum_of_digits, solution, Arena, List};

#[test]
fn validate() {
    let arena1 = Arena::new();
    let l1 = get_random_list(&arena1);
    let arena2 = Arena::new();
    let l2 = get_random_list(&arena2);
    let original_result = unsafe { original_solution(l1, l2) };
    let result = unsafe { solution(l1, l2) };

    assert_eq!(original_result, result);
}
unsafe fn original_solution(mut l1: *const List, mut l2: *const List) -> u32 {
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
