#[cfg(test)]
mod tests;

pub const HASH_MAP_SIZE: usize = 32 * 1024 * 1024 - 5;
const NUMBER_OF_LOOKUPS: usize = 1024 * 1024;

const UNUSED: i32 = i32::MAX;
pub struct HashMapT {
    m_vector: Vec<i32>,
    n_buckets: usize,
}
impl HashMapT {
    pub fn new(size: usize) -> HashMapT {
        HashMapT {
            m_vector: vec![UNUSED; size],
            n_buckets: size,
        }
    }

    pub fn insert(&mut self, val: i32) -> bool {
        let bucket: usize = val as usize % self.n_buckets;
        if self.m_vector[bucket] == UNUSED {
            self.m_vector[bucket] = val;
            true
        } else {
            false
        }
    }

    pub fn find(&self, val: i32) -> bool {
        let bucket = val as usize % self.n_buckets;
        self.m_vector[bucket] != UNUSED
    }
}

fn get_sum_of_digits(mut n: i32) -> i32 {
    let mut sum = 0;
    while n != 0 {
        sum += n % 10;
        n /= 10;
    }
    sum
}

pub fn solution(hash_map: &HashMapT, lookups: &[i32]) -> i32 {
    let mut result = 0;

    for &val in lookups {
        if hash_map.find(val) {
            result += get_sum_of_digits(val);
        }
    }

    result
}

pub fn init(hash_map: &mut HashMapT) -> Vec<i32> {
    use rand::prelude::*;
    let mut generator = thread_rng();

    for _ in 0..HASH_MAP_SIZE {
        hash_map.insert(generator.gen());
    }

    let mut lookups = Vec::with_capacity(NUMBER_OF_LOOKUPS);
    lookups.resize_with(NUMBER_OF_LOOKUPS, || generator.gen());

    lookups
}
