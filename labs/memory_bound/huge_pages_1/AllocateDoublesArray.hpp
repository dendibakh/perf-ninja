#include <iostream>
#include <memory>
#include <string.h>

#if defined(__linux__) || defined(__linux) || defined(linux) ||                \
    defined(__gnu_linux__)
#define ON_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define ON_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define ON_WINDOWS
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
  if (!OpenProcessToken(GetCurrentProcess(), desired_access, &handle))
    throw std::runtime_error{"OpenProcessToken failed"};
  return ret_t{handle, +handle_cleanup};
}

inline auto getUserToken() {
  auto proc_token = detail::openProcToken(TOKEN_QUERY);

  // Probe the buffer size reqired for PTOKEN_USER structure
  DWORD dwbuf_sz = 0;
  if (!GetTokenInformation(proc_token.get(), TokenUser, nullptr, 0,
                              &dwbuf_sz) &&
      (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    throw std::runtime_error{"GetTokenInformation failed"};

  // Retrieve the token information in a TOKEN_USER structure
  constexpr auto deleter = [](PTOKEN_USER ptr) { free(ptr); };
  PTOKEN_USER ptr = (PTOKEN_USER)malloc(dwbuf_sz);
  std::unique_ptr<TOKEN_USER, decltype(deleter)> user_token{ptr, deleter};
  if (!GetTokenInformation(proc_token.get(), TokenUser, user_token.get(),
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

  if (!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME,
                               &priv_token.Privileges->Luid))
    throw std::runtime_error{"LookupPrivilegeValue failed"};

  if (!AdjustTokenPrivileges(proc_token.get(), FALSE, &priv_token, 0,
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

#include <errno.h>
#include <fcntl.h>
#include <linux/kernel-page-flags.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define fail(...) do { /* fprintf(stderr, __VA_ARGS__); */ hugePagesFailed = true; return;} while (0)

// normal page, 4KiB
#define PAGE_SIZE (1 << 12)
// huge page, 2MiB
#define HPAGE_SIZE (1 << 21)

// See <https://www.kernel.org/doc/Documentation/vm/pagemap.txt> for
// format which these bitmasks refer to
#define PAGEMAP_PRESENT(ent) (((ent) & (1ull << 63)) != 0)
#define PAGEMAP_PFN(ent) ((ent) & ((1ull << 55) - 1))

static void check_huge_page(void* ptr);


constexpr static std::size_t huge_page_size = 1 << 21;
size_t round_to_huge_page_size(size_t n) {
  return (((n - 1) / huge_page_size) + 1) * huge_page_size;
}

size_t round_to_huge_page_n(size_t n) {
  return (((n - 1) / huge_page_size) + 1);
}


// [HERE IS NICE TUTORIAL HOW TO COOK HUGE PAGE TABLES]
// https://mazzo.li/posts/check-huge-page.html

// Allocate an array of doubles of size `size`, return it as a
// std::unique_ptr<double[], D>, where `D` is a custom deleter type
bool hugePagesFailed = false;

#include <functional>

inline auto allocateDoublesArray(size_t size) {
  // Allocate memory
  using delete_t = std::function<void(double*)>;

  delete_t deleter = [size](double *ptr) { delete [] ptr; };


  if (hugePagesFailed) {
    double *alloc = new double[size];
    return std::unique_ptr<double[], decltype(deleter)>(static_cast<double*>(alloc),
                                                        std::move(deleter));
  }

  // [Round to Huge Page Tablle size]
  size *= sizeof(double);
  size = round_to_huge_page_size(size);

  // [Request Huge Page Tabel]
  void* alloc = aligned_alloc(huge_page_size, size);
  if (alloc == NULL)
    throw std::bad_alloc{};
  madvise(alloc, size, MADV_HUGEPAGE);

  // [Allocate and check hugeness of page tables]
  char* it = (char*) alloc;
  for (char* end = it + size; !hugePagesFailed && it < end; it += huge_page_size) {
    memset(it, 0, 1);
    check_huge_page(it);
  }

  delete_t deleterok = [size](double *ptr) { munmap(ptr, size); };
  return std::unique_ptr<double[], decltype(deleter)>(static_cast<double*>(alloc),
                                                      std::move(deleterok));  
  
  // remember to cast the pointer to double* if your allocator returns void*

  // Deleters can be conveniently defined as lambdas, but you can explicitly
  // define a class if you're not comfortable with the syntax
  
  // The above is equivalent to:
  // return std::make_unique<double[]>(size);
  // The more verbose version is meant to demonstrate the use of a custom
  // (potentially stateful) deleter
}


// Checks if the page pointed at by `ptr` is huge. Assumes that `ptr` has already
// been allocated.
static void check_huge_page(void* ptr) {
  int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
  if (pagemap_fd < 0) {
    fail("could not open /proc/self/pagemap: %s", strerror(errno));
  }
  int kpageflags_fd = open("/proc/kpageflags", O_RDONLY);
  if (kpageflags_fd < 0) {
    fail("could not open /proc/kpageflags: %s", strerror(errno));
  }

  // each entry is 8 bytes long
  uint64_t ent;
  if (pread(pagemap_fd, &ent, sizeof(ent), ((uintptr_t) ptr) / PAGE_SIZE * 8) != sizeof(ent)) {
    fail("could not read from pagemap\n");
  }

  if (!PAGEMAP_PRESENT(ent)) {
    fail("page not present in /proc/self/pagemap, did you allocate it?\n");
  }
  if (!PAGEMAP_PFN(ent)) {
    fail("page frame number not present, run this program as root\n");
  }

  uint64_t flags;
  if (pread(kpageflags_fd, &flags, sizeof(flags), PAGEMAP_PFN(ent) << 3) != sizeof(flags)) {
    fail("could not read from kpageflags\n");
  }

  if (!(flags & (1ull << KPF_THP))) {
    fail("could not allocate huge page\n");
  }

  if (close(pagemap_fd) < 0) {
    fail("could not close /proc/self/pagemap: %s", strerror(errno));
  }
  if (close(kpageflags_fd) < 0) {
    fail("could not close /proc/kpageflags: %s", strerror(errno));
  }
}