#include "solution.hpp"

#include <cassert>
#include <mutex>

class BucketMapper {
  public:
    BucketMapper() {
      std::call_once(lookup_table_initialized_, [this](){
        for (size_t i = 0; i < lookup_table_.size(); i++) {
          if (i < 13)                  lookup_table_[i] =  0; // 13
          else if (i >= 13 && i < 29)  lookup_table_[i] =  1; // 16
          else if (i >= 29 && i < 41)  lookup_table_[i] =  2; // 12
          else if (i >= 41 && i < 53)  lookup_table_[i] =  3; // 12
          else if (i >= 53 && i < 71)  lookup_table_[i] =  4; // 18
          else if (i >= 71 && i < 83)  lookup_table_[i] =  5; // 12
          else if (i >= 83)            lookup_table_[i] =  6; // 17
        }
      });
    }

    size_t map(size_t v) {
      assert(v < lookup_table_.size());
      return lookup_table_[v];
    }
  private:
    inline static std::once_flag lookup_table_initialized_;
    inline static std::array<size_t, 100> lookup_table_;
};

std::array<size_t, NUM_BUCKETS> histogram(const std::vector<int> &values) {
  std::array<size_t, NUM_BUCKETS> retBuckets{0};
  BucketMapper mapper;
  for (auto v : values) {
    retBuckets[mapper.map(v)]++;
  }
  return retBuckets;
}
