
// https://en.cppreference.com/w/c/atomic

ATOMIC_CAS // cannot swap struct, need to be force convert to _Atomic(xxx) (eg. struct {uint8_t, uint8_t} ==> uint16_t)
ATOMIC_FAA

ATOMIC_STOR
ATOMIC_LOAD

ATOMIC_FLAG
ATOMIC_TAS // test and set
ATOMIC_CLR // clear

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
    && !defined(__STDC_NO_ATOMICS__)
#   include <stdatomic.h>
#elif defined(_MSC_VER) && (_MSC_VER >= 1800) /* Visual Studio 2013 */ \
    && (defined(_M_X64) || defined(_M_IX86))
#   define USE_TEMPORARY_MSVC_WORKAROUND 1
#else
#   error Atomic operations are not supported on your platform
#endif /* __STDC_NO_ATOMICS__ */


#ifdef __STDC_NO_ATOMICS__

#define ATOM_INT int
#define ATOM_POINTER void *
#define ATOM_SIZET size_t
#define ATOM_ULONG unsigned long
#define ATOM_INIT(ptr, v) (*(ptr) = v)
#define ATOM_LOAD(ptr) (*(ptr))
#define ATOM_STORE(ptr, v) (*(ptr) = v)
#define ATOM_CAS(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define ATOM_CAS_POINTER(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define ATOM_FINC(ptr) __sync_fetch_and_add(ptr, 1)
#define ATOM_FDEC(ptr) __sync_fetch_and_sub(ptr, 1)
#define ATOM_FADD(ptr,n) __sync_fetch_and_add(ptr, n)
#define ATOM_FSUB(ptr,n) __sync_fetch_and_sub(ptr, n)
#define ATOM_FAND(ptr,n) __sync_fetch_and_and(ptr, n)

#else

#include <stdatomic.h>

#define ATOM_INT atomic_int
#define ATOM_POINTER atomic_uintptr_t
#define ATOM_SIZET atomic_size_t
#define ATOM_ULONG atomic_ulong
#define ATOM_INIT(ref, v) atomic_init(ref, v)
#define ATOM_LOAD(ptr) atomic_load(ptr)
#define ATOM_STORE(ptr, v) atomic_store(ptr, v)
#define ATOM_CAS(ptr, oval, nval) atomic_compare_exchange_weak(ptr, &(oval), nval)
#define ATOM_CAS_POINTER(ptr, oval, nval) atomic_compare_exchange_weak(ptr, &(oval), nval)
#define ATOM_FINC(ptr) atomic_fetch_add(ptr, 1)
#define ATOM_FDEC(ptr) atomic_fetch_sub(ptr, 1)
#define ATOM_FADD(ptr,n) atomic_fetch_add(ptr, n)
#define ATOM_FSUB(ptr,n) atomic_fetch_sub(ptr, n)
#define ATOM_FAND(ptr,n) atomic_fetch_and(ptr, n)

#endif

#define SYNC_SWAP(addr,x)         ({ typeof(*(addr)) _old = *(addr); *(addr)  = (x); _old; })
#define SYNC_CAS(addr,old,x)      ({ typeof(*(addr)) _old = *(addr); *(addr)  = (x); _old; })
//#define SYNC_CAS(addr,old,x)    ({ typeof(*(addr)) _old = *(addr); if ((old) == _old) { *(addr)  = (x); } _old; })
#define SYNC_ADD(addr,n)          ({ typeof(*(addr)) _old = *(addr); *(addr) += (n); _old; })
#define SYNC_FETCH_AND_OR(addr,x) ({ typeof(*(addr)) _old = *(addr); *(addr) |= (x); _old; })

#define ATOMIC_VAR_INIT(value)      (value)
#define atomic_init(object, value)  (void)(*(object) = (value))

#ifdef __GNUC__
#define atomic_store(object, desired)   (void)( *(volatile typeof(*(object)) *)(object) = (desired) )
#define atomic_load(object)             *(volatile typeof(*(object)) *)(object)
#else
#define atomic_store(object, desired)   (void)(*(object) = (desired))
#define atomic_load(object)             *(object)
#endif

#define ATOMIC_FLAG_INIT { 0 }

