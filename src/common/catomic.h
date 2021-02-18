
// https://en.cppreference.com/w/c/atomic

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
    && !defined(__STDC_NO_ATOMICS__)
    #define _USE_STD
#elif defined(_MSC_VER) && (_MSC_VER >= 1800) /* Visual Studio 2013 */ \
    && (defined(_M_X64) || defined(_M_IX86))
    #define USE_MSVC
#elif defined(__clang__)
    #define USE_CLANG
#elif defined(__GNUC__)
    #define USE_GCC
#else
    #define _USE_NONE
#endif /* __STDC_NO_ATOMICS__ */

// #undef _USE_STD
// #define _USE_NONE

#if defined(_USE_STD)
    #include <stdatomic.h>
    // var
    #define ATOMIC_VAR(_type) _Atomic(_type)
    // #define ATOMIC_VAR_INIT(_val)
    #define ATOMIC_VAR_CAS(_ptr, _optr, _nval) atomic_compare_exchange_weak((_ptr), (_optr), (_nval))
    #define ATOMIC_VAR_FAA(_ptr, _val) atomic_fetch_add((_ptr), (_val))
    #define ATOMIC_VAR_STOR(_ptr, _val) atomic_store((_ptr), (_val))
    #define ATOMIC_VAR_LOAD(_ptr) atomic_load(_ptr)
    // flag
    #define ATOMIC_FLAG volatile atomic_flag
    // #define ATOMIC_FLAG_INIT
    #define ATOMIC_FLAG_TAS(_ptr) atomic_flag_test_and_set(_ptr)
    #define ATOMIC_FLAG_CLR(_ptr) atomic_flag_clear(_ptr)
#elif defined(_USE_NONE)
    // var
    #define ATOMIC_VAR(_type) _type
    #define ATOMIC_VAR_INIT(_val) (_val)
    #define ATOMIC_VAR_CAS(_ptr, _optr, _nval) ({ bool __ret = false; if (*((uintptr_t *)_optr) == *(uintptr_t *)(_ptr)) { *(_ptr) = (_nval); __ret = true; } __ret; })
    #define ATOMIC_VAR_FAA(_ptr, _val) ({ typeof(*(_ptr)) __oval = *(_ptr); *(_ptr) += (_val); __oval; })
    #define ATOMIC_VAR_STOR(_ptr, _val) (void)(*(_ptr) = (_val))
    #define ATOMIC_VAR_LOAD(_ptr) (*(_ptr))
    // flag
    #define ATOMIC_FLAG volatile bool
    #define ATOMIC_FLAG_INIT (false)
    #define ATOMIC_FLAG_TAS(_ptr) ({ bool __oval = *(_ptr); if (!__oval) { *(_ptr) = true; } __oval; })
    #define ATOMIC_FLAG_CLR(_ptr) (void)({ *(_ptr) = false; })
#endif
