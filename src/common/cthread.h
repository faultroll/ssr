
// https://en.cppreference.com/w/c/thread

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
    && !defined(__STDC_NO_THREADS__)
#   include <threads.h>
#elif defined(_MSC_VER) && (_MSC_VER >= 1800) /* Visual Studio 2013 */ \
    && (defined(_M_X64) || defined(_M_IX86))
#   define USE_TEMPORARY_MSVC_WORKAROUND 1
#else
#   error Atomic operations are not supported on your platform
#endif /* __STDC_NO_THREADS__ */

THREAD_TLS // thread local storage
// _Thread_local        // C11
// __thread struct      // GCC and Clang
// __declspec(thread)   // MSVC


THREAD_YIELD
thread_yield_fast thrd_yield // C11
thread_yield_fast sched_yield // pthread
thread_yield_fast SwitchToThread // MSVC
thread_yield_fast taskDelay // rtos

thread_yield_slow nanosleep




THREAD_ONCE

TPOOL_XXX // thread pool

// mutual exclusion

#if defined(_USE_PTHREAD)

#elif defined(_USE_STD)

// mtx_t

#elif defined(_USE_FUTEX)

#elif defined(_USE_ATOMIC)
// node state
enum {
    NST_EMPTY,
    NST_BUSY, // mutex, preempt
    NST_FULL,
};
// lock type
enum {
    NST_PUSH,
    NST_POP,
};
// static ATOMIC_VAR(int) g_pplock_cnt = 0;
#define NST_PP_LOCK(_lock, _type) do { const int __oval = (NST_PUSH == _type) ? NST_EMPTY : NST_FULL, __nval = NST_BUSY; do {} while (!ATOMIC_VAR_CAS(&(_lock), __oval, __nval)); } while (0)
#define NST_PP_UNLOCK(_lock, _type) do { const int __nval = (NST_PUSH == _type) ? NST_FULL : NST_EMPTY; ATOMIC_VAR_STOR(&(_lock), __nval); } while (0)
// api
#define NST_LOCK ATOMIC_VAR(int)
#define NST_LOCK_INIT(_lock) ATOMIC_VAR_STOR(&(_lock), NST_EMPTY)

// http://blog.kongfy.com/2017/01/%E7%94%A8%E6%88%B7%E6%80%81%E5%90%8C%E6%AD%A5%E4%B9%8B%E8%87%AA%E6%97%8B%E9%94%81/
static ATOMIC_FLAG g_spin = ATOMIC_FLAG_INIT;
#define SPIN_LOCK(_lock) do { while (!ATOMIC_FLAG_TAS(&(_lock))) {} } while (0)
#define SPIN_UNLOCK(_lock) do { ATOMIC_FLAG_CLR(&(_lock)); } while (0)

#endif
