
#pragma once

#include <stdint.h>
#include <chrono>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  typedef HMODULE LibHandle;
  static inline LibHandle lib_open(const char* path) { return LoadLibraryA(path); }
  static inline void*     lib_sym (LibHandle h, const char* name){ return (void*)GetProcAddress(h, name); }
  static inline void      lib_close(LibHandle h){ if(h) FreeLibrary(h); }
#else
  #include <dlfcn.h>
  typedef void* LibHandle;
  static inline LibHandle lib_open(const char* path) { return dlopen(path, RTLD_NOW | RTLD_LOCAL); }
  static inline void*     lib_sym (LibHandle h, const char* name){ return dlsym(h, name); }
  static inline void      lib_close(LibHandle h){ if(h) dlclose(h); }
#endif

static inline std::chrono::steady_clock::time_point lib_now() 
{ 
  return std::chrono::steady_clock::now(); 
}
