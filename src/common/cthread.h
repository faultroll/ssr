
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
THREAD_YIELD
THREAD_ONCE

