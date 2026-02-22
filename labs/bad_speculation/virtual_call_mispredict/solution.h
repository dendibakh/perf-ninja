#include <cstdint>
#include <vector>
#include <memory>

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

struct BaseClass {
    BaseClass(size_t offset = 0) : offset_(offset) {}
    virtual ~BaseClass() = default;

    void handle(std::size_t& data) const {
      data += offset_;
    }
    size_t offset_;
};

struct ClassA : public BaseClass {
  ClassA() : BaseClass(1) {}
};

struct ClassB : public BaseClass {
  ClassB() : BaseClass(2) {}
};

struct ClassC : public BaseClass {
  ClassC() : BaseClass(3) {}
};

using InstanceArray = std::vector<std::unique_ptr<BaseClass>>;

void generateObjects(InstanceArray& array);
void invoke(InstanceArray& array, std::size_t& data);
