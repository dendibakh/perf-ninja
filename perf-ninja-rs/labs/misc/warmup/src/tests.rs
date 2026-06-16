use crate::solution;

#[test]
fn validate() {
    const N: usize = 1000;
    let mut arr = [0i32; N];
    for i in 0..N {
        arr[i] = i as i32 + 1i32;
    }

    let result = solution(&arr, N);
    assert_eq!(result as usize, (N * (N + 1)) / 2, "Validation Failed");
}
