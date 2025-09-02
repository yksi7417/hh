
#pragma once

#include <stdint.h>

#include <chrono>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
typedef HMODULE LibHandle;
// Performance critical: inline function for loading DLLs (hot path)
static inline LibHandle lib_open(const char* path) {
    return LoadLibraryA(path);
}
// Performance critical: inline function for symbol lookup (hot path)
static inline void* lib_sym(LibHandle h, const char* name) {
    return (void*)GetProcAddress(h, name);
}
// Performance critical: inline function for DLL cleanup (hot path)
static inline void lib_close(LibHandle h) {
    if (h)
        FreeLibrary(h);
}
#else
#include <dlfcn.h>
typedef void* LibHandle;
// Performance critical: inline function for loading shared objects (hot path)
static inline LibHandle lib_open(const char* path) {
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
}
// Performance critical: inline function for symbol lookup (hot path)
static inline void* lib_sym(LibHandle h, const char* name) {
    return dlsym(h, name);
}
// Performance critical: inline function for shared object cleanup (hot path)
static inline void lib_close(LibHandle h) {
    if (h)
        dlclose(h);
}
#endif

// Performance critical: inline function for high-resolution timing (hot path)
static inline std::chrono::steady_clock::time_point lib_now() {
    return std::chrono::steady_clock::now();
}
