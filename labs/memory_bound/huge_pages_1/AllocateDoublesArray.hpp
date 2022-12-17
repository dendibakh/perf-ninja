#include <iostream>
#include <memory>

#if defined(__linux__) || defined(__linux) || defined(linux) ||                \
    defined(__gnu_linux__)
#define ON_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define ON_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define ON_WINDOWS
#endif

#if defined(ON_MACOS)
#include <mach/mach_vm.h>
#include <mach/mach.h>
#include <map>
#include <new>
#include <vector>

#define NUMBER_OF_SUPERPAGES 10
#define SUPERPAGE_SIZE (NUMBER_OF_SUPERPAGES * 16 * 1024 * 1024) // Allocate 10 superpages of 16MB each
 
template <typename T>
class PreallocatedAllocator {
 public:
  // Type definitions
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef long long int size_type;
  typedef unsigned long long int block_type;
  typedef std::ptrdiff_t difference_type;

  // Rebind allocator to type U
  template <typename U>
  struct rebind {
    typedef PreallocatedAllocator<U> other;
  };

  // Return address of values
  pointer address(reference value) const noexcept { return &value; }
  const_pointer address(const_reference value) const noexcept { return &value; }

  /* Constructors and destructor
   * - The constructor preallocates a large block of memory
   * - The destructor deallocates the block
   */
  explicit PreallocatedAllocator(size_type num_elements) :
      num_elements_(num_elements),
      block_size_(num_elements * sizeof(T))
    {
      // Allocate the block of memory using mach_vm_allocate
      kern_return_t ret;
      ret = mach_vm_allocate(mach_task_self(), &block_, block_size_, VM_FLAGS_ANYWHERE | SUPERPAGE_SIZE_ANY);
      if (ret != KERN_SUCCESS) {
        throw std::bad_alloc();
      }
  }
  PreallocatedAllocator(const PreallocatedAllocator&) = delete;
  template <typename U>
  PreallocatedAllocator(const PreallocatedAllocator<U>&) = delete;
  ~PreallocatedAllocator() {
    // Deallocate the block of memory using mach_vm_deallocate
    kern_return_t ret;
    ret = vm_deallocate(mach_task_self(), block_, block_size_);
    if (ret != KERN_SUCCESS) {
      throw std::bad_alloc();
    }
  }

  // Return maximum number of elements that can be allocated
  size_type max_size() const noexcept { return num_elements_; }

  pointer allocate(size_type num, const void* = 0) {
    // Check if there is enough available memory to satisfy the allocation request
    if (next_free_index + num > num_elements_) {
      // Return nullptr if there is not enough available memory
      throw std::bad_alloc();
      return nullptr;
    }
    // Calculate the index of the block within the memory block
    size_type index = next_free_index;
    // Update next_free_index to point to the next available index
    next_free_index += num;
    // Return a pointer to the found block
    return static_cast<pointer>((void*)(block_ + index * sizeof(T)));
  }
  
  // Deallocate storage p of deleted elements
  void deallocate(pointer p, size_type num) {
    // Calculate the offset of the block within the preallocated block
    block_type offset = (block_type)p - block_;
    // Calculate the index of the block within the memory block
    size_type index = offset / sizeof(T);
    // Update next_free_index to point to the deallocated block
    next_free_index = index;
  }

  // Initialize elements of allocated storage p with value value
  void construct(pointer p, const T& value) {
    // Initialize memory with placement new
    new((void*)p) T(value);
  }

  // Destroy elements of initialized storage p
  void destroy(pointer p) {
    // Call the destructor of T
    p->~T();
  }

private:
  // The preallocated block of memory
  block_type block_;
  // The size of the preallocated block, in bytes
  size_type block_size_;
  // The number of elements that can be stored in the preallocated block
  size_type num_elements_;
  size_type next_free_index = 0;
};
#endif

#if defined(ON_LINUX)

// HINT: allocate huge pages using mmap/munmap
// NOTE: See HugePagesSetupTips.md for how to enable huge pages in the OS
#include <sys/mman.h>

#elif defined(ON_WINDOWS)

// HINT: allocate huge pages using VirtualAlloc/VirtualFree
// NOTE: See HugePagesSetupTips.md for how to enable huge pages in the OS.
// The following boilerplate has been included to save you some time. You'll
// need to call `setRequiredPrivileges()` once per program run to tell the OS
// that you want to use huge pages. Additionally, you'll need to set the
// relevant user account privilege. The following code should do this for you,
// provided you run it as admin (run as admin -> reboot -> you're set to run as
// user).
// HINT: A good way to execute a function once per program run is to call it in
// the initialization of a function static variable.

#define UNICODE
#define _UNICODE

#include <windows.h>

#include <Sddl.h>
#include <ntsecapi.h>
#include <ntstatus.h>

// Based on
// https://stackoverflow.com/questions/42354504/enable-large-pages-in-windows-programmatically
namespace detail {
inline void InitLsaString(PLSA_UNICODE_STRING LsaString, LPWSTR String) {
  DWORD StringLength;

  if (String == NULL) {
    LsaString->Buffer = NULL;
    LsaString->Length = 0;
    LsaString->MaximumLength = 0;
    return;
  }

  StringLength = wcslen(String);
  LsaString->Buffer = String;
  LsaString->Length = (USHORT)StringLength * sizeof(WCHAR);
  LsaString->MaximumLength = (USHORT)(StringLength + 1) * sizeof(WCHAR);
}

inline auto openProcToken(DWORD desired_access) {
  using handle_t = std::pointer_traits<HANDLE>::element_type;
  constexpr auto handle_cleanup = [](HANDLE ptr) { CloseHandle(ptr); };
  using ret_t = std::unique_ptr<handle_t, decltype(+handle_cleanup)>;

  HANDLE handle{};
  if (not OpenProcessToken(GetCurrentProcess(), desired_access, &handle))
    throw std::runtime_error{"OpenProcessToken failed"};
  return ret_t{handle, +handle_cleanup};
}

inline auto getUserToken() {
  auto proc_token = detail::openProcToken(TOKEN_QUERY);

  // Probe the buffer size reqired for PTOKEN_USER structure
  DWORD dwbuf_sz = 0;
  if (not GetTokenInformation(proc_token.get(), TokenUser, nullptr, 0,
                              &dwbuf_sz) and
      (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    throw std::runtime_error{"GetTokenInformation failed"};

  // Retrieve the token information in a TOKEN_USER structure
  constexpr auto deleter = [](PTOKEN_USER ptr) { free(ptr); };
  PTOKEN_USER ptr = (PTOKEN_USER)malloc(dwbuf_sz);
  std::unique_ptr<TOKEN_USER, decltype(deleter)> user_token{ptr, deleter};
  if (not GetTokenInformation(proc_token.get(), TokenUser, user_token.get(),
                              dwbuf_sz, &dwbuf_sz))
    throw std::runtime_error{"GetTokenInformation failed"};

  return user_token;
}

inline void adjustAccountPrivilege() {
  auto user_token = getUserToken();

  LSA_OBJECT_ATTRIBUTES obj_attrib{};
  LSA_HANDLE policy_handle;
  if (LsaOpenPolicy(nullptr, &obj_attrib,
                    POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
                    &policy_handle))
    throw std::runtime_error{"LsaOpenPolicy failed"};

  LSA_UNICODE_STRING privilege_string;
  InitLsaString(&privilege_string, SE_LOCK_MEMORY_NAME);
  if (LsaAddAccountRights(policy_handle, user_token->User.Sid,
                          &privilege_string, 1))
    throw std::runtime_error{"LsaAddAccountRights failed"};
}

inline bool enableProcPrivilege() {
  auto proc_token = openProcToken(TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES);

  TOKEN_PRIVILEGES priv_token{};
  priv_token.PrivilegeCount = 1;
  priv_token.Privileges->Attributes = SE_PRIVILEGE_ENABLED;

  if (not LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME,
                               &priv_token.Privileges->Luid))
    throw std::runtime_error{"LookupPrivilegeValue failed"};

  if (not AdjustTokenPrivileges(proc_token.get(), FALSE, &priv_token, 0,
                                nullptr, 0))
    throw std::runtime_error{"AdjustTokenPrivileges failed"};

  if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    return false;
  else
    return true;
}
} // namespace detail

inline bool setRequiredPrivileges() {
  if (detail::enableProcPrivilege())
    return true;
  else {
    std::clog << "It seems that your user account does not have the privilege "
                 "required to allocate huge pages. The program will now "
                 "attempt to give you this privilege. This requires the "
                 "program to be run as admin."
              << std::endl;
    detail::adjustAccountPrivilege();
    std::clog << "The required privilege has been set successfully. Please log "
                 "out and log back in for the changes to take effect. This "
                 "program will now terminate."
              << std::endl;
    std::terminate();
    return false;
  }
}

#endif

#if defined(ON_MACOS)

PreallocatedAllocator<double> GlobalAllocator (SUPERPAGE_SIZE);

// Allocate an array of doubles of size `size`, return it as a
// std::unique_ptr<double[], D>, where `D` is a custom deleter type
inline auto allocateDoublesArray(size_t size) {
 // Allocate memory from the huge page just for size passed in
  double *alloc = GlobalAllocator.allocate(size);

  // remember to cast the; pointer to double* if your allocator returns void*
  // Deleters can be conveniently defined as lambdas, but you can explicitly
  // define a class i you're not comfortable with the syntax
  auto deleter = [/* state = ... */size](double *ptr) { GlobalAllocator.deallocate(ptr, size);};

  return std::unique_ptr<double[], decltype(deleter)>(alloc,
                                                      std::move(deleter));
} 
#else
// Allocate an array of doubles of size `size`, return it as a
// std::unique_ptr<double[], D>, where `D` is a custom deleter type
inline auto allocateDoublesArray(size_t size) {
  // Allocate memory
  double *alloc = new double[size];
  // remember to cast the pointer to double* if your allocator returns void*

  // Deleters can be conveniently defined as lambdas, but you can explicitly
  // define a class if you're not comfortable with the syntax
  auto deleter = [/* state = ... */](double *ptr) { delete[] ptr; };

  return std::unique_ptr<double[], decltype(deleter)>(alloc,
                                                      std::move(deleter));

  // The above is equivalent to:
  // return std::make_uniquePROT_READ, MAP_ANON | MAP_PRIVATE, VM_FLAGS_SUPERPAGE_SIZE_2MB vse version is meant to demonstrate the use of a custom
  // (potentially stateful) deleter
}
#endif