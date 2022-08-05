#ifndef MAPPEDFILE_HPP
#define MAPPEDFILE_HPP

#if defined(__linux__) || defined(__linux) || defined(linux) ||                \
    defined(__gnu_linux__)
#define ON_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define ON_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define ON_WINDOWS
#endif

#include <stdexcept>
#include <string>
#include <string_view>

#ifdef ON_LINUX
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef ON_WINDOWS
#include <windows.h>

#include <fileapi.h>
#include <memoryapi.h>
#include <winbase.h>
#endif

// This class is a RAII wrapper for a file mapping. Pass a string containing
// the name of the file to the constructor, and then access its contents as
// a std::string_view via getContents(). All the gnarly details of creating
// and destroying the mapping are taken care of. Portable to Windows and Linux.
class MappedFile {
public:
  MappedFile() : m_content{nullptr}, m_size{0} {}
  explicit inline MappedFile(const std::string &path);
  MappedFile(const MappedFile &) = delete;
  MappedFile &operator=(const MappedFile &) = delete;
  inline MappedFile(MappedFile &&other) noexcept;
  inline MappedFile &operator=(MappedFile &&other) noexcept;
  ~MappedFile() {
    if (m_content)
      dtorImpl();
  }

  [[nodiscard]] auto getContents() const -> std::string_view {
    return {m_content, m_size};
  }

private:
  inline void dtorImpl() noexcept;

  char *m_content;
  std::size_t m_size;

#ifdef ON_WINDOWS
  HANDLE m_file_handle;
  HANDLE m_mapping_handle;
#endif
};

#ifdef ON_LINUX
MappedFile::MappedFile(const std::string &path) {
  const int fd = open(path.c_str(), O_RDONLY | O_LARGEFILE);
  if (fd == -1)
    throw std::runtime_error{"Could not open the specified file"};
  struct stat sb {};
  if (fstat(fd, &sb))
    throw std::runtime_error{"Could not obtain file info"};
  m_size = static_cast<std::size_t>(sb.st_size);

  void *const mapped_addr = mmap(NULL, m_size, PROT_READ, MAP_PRIVATE, fd,
                                 0u); // NOLINT it's a C API...
  if (mapped_addr == MAP_FAILED)
    throw std::runtime_error{"Could not mmap file"};

  m_content = static_cast<char *>(mapped_addr);

  if (close(fd))
    throw std::runtime_error{"Could not close file after mmap"};
}

MappedFile::MappedFile(MappedFile &&other) noexcept
    : m_content{other.m_content}, m_size{other.m_size} {
  other.m_content = nullptr;
}

MappedFile &MappedFile::operator=(MappedFile &&other) noexcept {
  if (this != &other) {
    dtorImpl();
    m_content = other.m_content;
    m_size = other.m_size;
    other.m_content = nullptr;
  }
  return *this;
}

void MappedFile::dtorImpl() noexcept {
  if (m_content)
    munmap(m_content, m_size); // Ignore error - dtor shouldn't throw  we can't
                               // meaningfully handle this error anyway
}
#endif

#ifdef ON_WINDOWS
MappedFile::MappedFile(const std::string &path) {
  m_file_handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (m_file_handle == INVALID_HANDLE_VALUE)
    throw std::runtime_error{"Could not open file"};

  LARGE_INTEGER size{};
  if (not GetFileSizeEx(m_file_handle, &size)) {
    CloseHandle(m_file_handle);
    throw std::runtime_error{"Could not obtain file size"};
  }
  m_size = size.QuadPart;

  m_mapping_handle =
      CreateFileMappingA(m_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
  if (m_mapping_handle == NULL) {
    CloseHandle(m_file_handle);
    throw std::runtime_error{"Could not create file mapping"};
  }

  const auto map_content =
      MapViewOfFile(m_mapping_handle, FILE_MAP_READ, 0, 0, 0);
  if (map_content == NULL) {
    CloseHandle(m_mapping_handle);
    CloseHandle(m_file_handle);
    throw std::runtime_error{"Could not map file into process address space"};
  }
  m_content = static_cast<char *>(map_content);
}

MappedFile::MappedFile(MappedFile &&other) noexcept
    : m_content{other.m_content}, m_size{other.m_size},
      m_file_handle{other.m_file_handle}, m_mapping_handle{
                                              other.m_mapping_handle} {
  other.m_content = nullptr;
}

MappedFile &MappedFile::operator=(MappedFile &&other) noexcept {
  if (this != &other) {
    dtorImpl();
    m_content = other.m_content;
    m_size = other.m_size;
    m_file_handle = other.m_file_handle;
    m_mapping_handle = other.m_mapping_handle;
    other.m_content = nullptr;
  }
  return *this;
}

void MappedFile::dtorImpl() noexcept {
  if (not m_content)
    return;
  UnmapViewOfFile(m_content);
  CloseHandle(m_mapping_handle);
  CloseHandle(m_file_handle);
}
#endif
#endif // MAPPEDFILE_HPP