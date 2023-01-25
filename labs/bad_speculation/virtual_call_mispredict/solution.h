#include <cstdint>
#include <vector>
#include <memory>

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

struct BaseClass {
    virtual ~BaseClass() = default;
    int a;
    void handle(std::size_t& data) const {
        data += a;
    }
};

struct ClassA : public BaseClass {
    int a=1;
};

struct ClassB : public BaseClass {
    int a = 2;
};

struct ClassC : public BaseClass {
    int a = 3;
};

using InstanceArray = std::vector<std::unique_ptr<BaseClass>>;

void generateObjects(InstanceArray& array);
void invoke(InstanceArray& array, std::size_t& data);
