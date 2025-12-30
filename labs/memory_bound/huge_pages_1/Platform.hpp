#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(__linux__) || defined(__linux) || defined(linux) ||                \
    defined(__gnu_linux__)
#define ON_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define ON_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define ON_WINDOWS
#endif

#if defined(ON_LINUX) || defined(ON_WINDOWS)
#define PREFETCH_ENABLED
#endif

#ifdef PREFETCH_ENABLED
#define PREFETCH(addr) __builtin_prefetch((addr))
#define PREFETCHW(addr) __builtin_prefetch((addr), 1)
#else
#define PREFETCH(addr)
#define PREFETCHW(addr)
#endif

#endif
