#include <vector>
#include <limits>

static constexpr std::size_t HASH_MAP_SIZE = 32 * 1024 * 1024 - 5;
static constexpr std::size_t NUMBER_OF_LOOKUPS = 1024 * 1024;

// Force CI to compile solution
#define SOLUTION

#ifdef SOLUTION

#ifdef _MSC_VER 
// This means windows as well
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Winnt.h>
#undef VC_EXTRALEAN
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#define PREFETCH(a) PreFetchCacheLine(PF_TEMPORAL_LEVEL_1, (void *)(a))
#else
// For us this means GNU or Clang
#define PREFETCH(a) __builtin_prefetch((void *)(a))
#endif
#endif

class hash_map_t {
    static constexpr int UNUSED = std::numeric_limits<int>::max();
    std::vector<int> m_vector;
    std::size_t N_Buckets;
public:
    hash_map_t(std::size_t size) : m_vector(size, UNUSED), N_Buckets(size) {}

    bool insert(int val) {
        int bucket = val % N_Buckets;
        if (m_vector[bucket] == UNUSED) {
            m_vector[bucket] = val;
            return true;
        }
        return false;
    }

    bool find(int val) const {
        int bucket = val % N_Buckets;
        return m_vector[bucket] != UNUSED;
    }

#ifdef SOLUTION

    void prefetch(int val) const {
        const int bucket = val % N_Buckets;
        PREFETCH(m_vector.data() + bucket);
    }

#endif

};

void init(hash_map_t* hash_map, std::vector<int>& lookups);
int solution(const hash_map_t* hash_map, const std::vector<int>& lookups);