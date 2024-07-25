#include <cstdint>
#include <vector>
#include <memory>
#include <variant>

// Assume this constant never changes
constexpr std::size_t N = 64 * 1024;

template <typename CRTP_t>
struct BaseClass {
    //virtual ~BaseClass() = default;

    virtual void handle(std::size_t& data) const { static_cast<const CRTP_t&>(*this).handle(data); };
};

struct ClassA : public BaseClass<ClassA> {
    void handle(std::size_t& data) const override {
        data += 1;
    }
};

struct ClassB : public BaseClass<ClassB> {
    void handle(std::size_t& data) const override {
        data += 2;
    }
};

struct ClassC : public BaseClass<ClassC> {
    void handle(std::size_t& data) const override {
        data += 3;
    }
};

template<class T>
using U = typename std::unique_ptr<T>; 

using InstanceArray = std::vector<std::variant< U<ClassA>, U<ClassB>, U<ClassC> >>;

void generateObjects(InstanceArray& array);
void invoke(InstanceArray& array, std::size_t& data);
