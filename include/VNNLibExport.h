#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef VNNLIB_BUILD_DLL
    #define VNNLIB_API __declspec(dllexport)
  #else
    #define VNNLIB_API __declspec(dllimport)
  #endif
#else
  #if defined(__GNUC__) || defined(__clang__)
    #define VNNLIB_API __attribute__((visibility("default")))
  #else
    #define VNNLIB_API
  #endif
#endif