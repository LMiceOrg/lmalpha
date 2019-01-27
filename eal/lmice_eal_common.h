#ifndef LMICE_EAL_COMMON_H
#define LMICE_EAL_COMMON_H

#define UNREFERENCED_PARAM(param) (void)param
#define lmice_unreferenced_param6(x1, x2, x3, x4, x5, x6) \
  (void)(x1);                                             \
  (void)(x2);                                             \
  (void)(x3);                                             \
  (void)(x4);                                             \
  (void)(x5);                                             \
  (void)(x6);

#define lmice_unreferenced_param5(x1, x2, x3, x4, x5) \
  (void)(x1);                                         \
  (void)(x2);                                         \
  (void)(x3);                                         \
  (void)(x4);                                         \
  (void)(x5);

#define lmice_unreferenced_param4(x1, x2, x3, x4) \
  (void)(x1);                                     \
  (void)(x2);                                     \
  (void)(x3);                                     \
  (void)(x4);

#define lmice_unreferenced_param3(x1, x2, x3) \
  (void)(x1);                                 \
  (void)(x2);                                 \
  (void)(x3);

#define lmice_unreferenced_param2(x1, x2) \
  (void)(x1);                             \
  (void)(x2);

#define lmice_unreferenced_param1(x1) (void)(x1);

#define lmice_static_assert(X, func)                                         \
  {                                                                          \
    extern int __attribute__((error("assertion failure: '" #X "' at '" #func \
                                    "' not true"))) compile_time_check();    \
    ((X) ? 0 : compile_time_check()), 0;                                     \
  }

/* 32bit -64bit */
#if defined(__LP64__) || defined(_WIN64) || defined(__amd64) || \
    defined(__x86_64__) || defined(_M_X64)
#define arch64 1
#define arch32 0
#define declare_pad8_pointer(type, name) type *name
#define declare_pad8_pointer_array(type, name, n) type *name[n]
#else /* 32bit */
#define arch64 0
#define arch32 1
#define declare_pad8_pointer(type, name) \
  type *name;                            \
  uint32_t name##_pad8
#define declare_pad8_pointer_array(type, name, n) \
  type *name[n];                                  \
  uint32_t name##pad8[n];
#endif

/** c version */
#if defined(__STDC__) && defined(__STDC_VERSION__)
#if __STDC_VERSION__ >= 199901L
#define EAL_STDC_VER 99
#elif __STDC_VERSION__ >= 199409L
#define EAL_STDC_VER 94
#endif
#elif defined(__STDC__)
#define EAL_STDC_VER 90
#endif

#if EAL_STDC_VER == 90
#define inline __inline
#endif

/**
  support clang gcc msvc
*/
#if defined(__clang__) /** clang */
#define forcepack(x) __attribute__((__aligned__((x))))
#define forceinline inline __attribute__((always_inline))
#define noexport __attribute__((visibility("hidden")))

#elif defined(__GNUC__) /** GCC */
#include <unistd.h>
#define forceinline __attribute__((always_inline)) inline static
#define forcepack(x) __attribute__((__aligned__((x))))
#define noexport __attribute__((visibility("hidden")))
#include "sys/types.h"

#if defined(__MINGW32__)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#endif

#elif defined(_MSC_VER) /** MSC */

#define forceinline static __forceinline
#define forcepack(x) __declspec(align(x))
#define noexport
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#else /** Other compiler */

#error(Unsupported compiler)
#endif

#endif /** LMICE_EAL_COMMON_H */
