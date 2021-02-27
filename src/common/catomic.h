
// https://en.cppreference.com/w/c/atomic

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
    && !defined(__STDC_NO_ATOMICS__)
    #define _USE_STD
#elif defined(__GNUC__)
    #define _USE_GCC
#elif defined(_MSC_VER)
    #define _USE_MSVC
#elif defined(__clang__)
    #define _USE_CLANG
#else
    #define _USE_NONE
#endif /* __STDC_NO_ATOMICS__ */

// #undef _USE_STD
// #define _USE_NONE

// ATOMIC_VAR should be smaller than WORDSIZE(maybe (u)intptr_t)
#if defined(_USE_STD)
    #include <stdatomic.h>
    // var
    #define ATOMIC_VAR(_type) _Atomic(_type)
    // #define ATOMIC_VAR_INIT(_val)
    #define ATOMIC_VAR_CAS(_ptr, _oval, _nval) ({ __typeof__(_oval) __stunt = _oval; atomic_compare_exchange_strong((_ptr), &__stunt, _nval); })
    #define ATOMIC_VAR_FAA(_ptr, _val) atomic_fetch_add((_ptr), (_val))
    #define ATOMIC_VAR_STOR(_ptr, _val) atomic_store((_ptr), (_val))
    #define ATOMIC_VAR_LOAD(_ptr) atomic_load(_ptr)
    // flag
    #define ATOMIC_FLAG volatile atomic_flag
    // #define ATOMIC_FLAG_INIT
    #define ATOMIC_FLAG_TAS(_ptr) atomic_flag_test_and_set(_ptr)
    #define ATOMIC_FLAG_CLR(_ptr) atomic_flag_clear(_ptr)
#elif defined(_USE_GCC)
    // use __sync or __atomic
    // var
    #define ATOMIC_VAR(_type) _type
    #define ATOMIC_VAR_INIT(_val) (_val)
    #define ATOMIC_VAR_CAS(_ptr, _oval, _nval) __sync_bool_compare_and_swap((_ptr), (_oval), (_nval))
    #define ATOMIC_VAR_FAA(_ptr, _val) __sync_fetch_and_add((_ptr), (_val))
    #define ATOMIC_VAR_STOR(_ptr, _val) (void)(*(_ptr) = (_val))
    #define ATOMIC_VAR_LOAD(_ptr) (*(_ptr))
    // flag
    #define ATOMIC_FLAG volatile bool
    #define ATOMIC_FLAG_INIT (false)
    #define ATOMIC_FLAG_TAS(_ptr) __sync_lock_test_and_set(_ptr, true)
    #define ATOMIC_FLAG_CLR(_ptr) __sync_lock_release(_ptr)
#elif defined(_USE_MSVC)
    // var
    #define ATOMIC_VAR(_type) _type
    #define ATOMIC_VAR_INIT(_val) (_val)
    #define ATOMIC_VAR_CAS(_ptr, _oval, _nval) (_InterlockedCompareExchange((_ptr), (_oval), (_nval)) == (_oval))
    #define ATOMIC_VAR_FAA(_ptr, _val) _InterlockedExchangeAdd((_ptr), (_val))
    #define ATOMIC_VAR_STOR(_ptr, _val) (void)(*(_ptr) = (_val))
    #define ATOMIC_VAR_LOAD(_ptr) (*(_ptr))
    // flag
    #define ATOMIC_FLAG volatile bool
    #define ATOMIC_FLAG_INIT (false)
    #define ATOMIC_FLAG_TAS(_ptr) _InterlockedExchange(_ptr, true)
    #define ATOMIC_FLAG_CLR(_ptr) _InterlockedExchange(_ptr, false)
#elif defined(_USE_CLANG)
    // #include <atomic_stub.h>
    // // var
    // #define ATOMIC_VAR(_type) _Atomic(_type)
    // // #define ATOMIC_VAR_INIT(_val)
    // #define ATOMIC_VAR_CAS(_ptr, _oval, _nval) __c11_atomic_compare_exchange_strong((_ptr), (_oval), (_nval), memory_order_seq_cst, memory_order_seq_cst)
    // #define ATOMIC_VAR_FAA(_ptr, _val) __c11_atomic_fetch_add((_ptr), (_val), memory_order_seq_cst)
    // #define ATOMIC_VAR_STOR(_ptr, _val) __c11_atomic_store((_ptr), (_val), memory_order_seq_cst)
    // #define ATOMIC_VAR_LOAD(_ptr) __c11_atomic_load((_ptr), memory_order_seq_cst)
    // // flag
    // #define ATOMIC_FLAG volatile atomic_flag
    // // #define ATOMIC_FLAG_INIT
    // #define ATOMIC_FLAG_TAS(_ptr) __c11_atomic_flag_test_and_set(_ptr)
    // #define ATOMIC_FLAG_CLR(_ptr) __c11_atomic_flag_clear(_ptr)
#elif defined(_USE_NONE)
    // var
    #define ATOMIC_VAR(_type) _type
    #define ATOMIC_VAR_INIT(_val) (_val)
    #define ATOMIC_VAR_CAS(_ptr, _oval, _nval) ({ bool __ret; if ((uintptr_t)_oval == *(uintptr_t *)(_ptr)) { *(_ptr) = (_nval); __ret = true; } else { __ret = false; } __ret; })
    #define ATOMIC_VAR_FAA(_ptr, _val) ({ __typeof__(*(_ptr)) __oval = *(_ptr); *(_ptr) += (_val); __oval; })
    #define ATOMIC_VAR_STOR(_ptr, _val) (void)(*(_ptr) = (_val))
    #define ATOMIC_VAR_LOAD(_ptr) (*(_ptr))
    // flag
    #define ATOMIC_FLAG volatile bool
    #define ATOMIC_FLAG_INIT (false)
    #define ATOMIC_FLAG_TAS(_ptr) ({ bool __oval = *(_ptr); if (!__oval) { *(_ptr) = true; } __oval; })
    #define ATOMIC_FLAG_CLR(_ptr) (void)({ *(_ptr) = false; })
#endif
