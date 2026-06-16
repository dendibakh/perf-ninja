#[cfg(test)]
mod tests;

pub fn solution(arr: &[i32], n: usize) -> i32 {
    let mut res = 0;
    for i in 0..n {
        res += arr[i];
    }
    res
}
