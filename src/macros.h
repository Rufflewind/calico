/** @file

    Miscellaneous utility macros.

    This header can be included multiple times.

*/

#include <string.h>                     /* for cal_pstrcmp */

/** Concatenate two tokens. */
#define cal_cat(x, y) calpriv_cat(x, y)
#define calpriv_cat(x, y) x ## y

/** Convert an expression into a string literal. */
#define cal_stringify(x) calpriv_stringify(x)
#define calpriv_stringify(x) #x

/** Calculate the minimum of two values. */
#define cal_min(x, y) ((x) < (y) ? (x) : (y))

/** Calculate the maximum of two values. */
#define cal_max(x, y) ((x) >= (y) ? (x) : (y))

/** Compare two values, returning a negative value if less than, positive
    value if greater than, and zero if equal. */
#define cal_cmp(ctx, x, y) (((y) < (x)) - ((x) < (y)))
/* note: for the btree implementation we have, this is noticeably more
   efficient than ((x) < (y) ? -1 : (y) < (x)) */

/** Compare two values pointed by the given pointers, returning a negative
    value if less than, positive value if greater than, and zero if equal. */
#define cal_pcmp(ctx, x, y) cal_cmp(ctx, *x, *y)

/** Compare two null-terminated strings pointed by the given pointers using
    `strcmp`. */
#define cal_pstrcmp(ctx, x, y) strcmp(*x, *y)

/** Expand to the first argument if the condition is 1, or the second argument
    if the condition is 0.  The behavior is unspecified if the condition is
    neither 1 nor 0.

    Usage: `cal_cond(condition)(value_if_1, value_if_0)`

*/
#define cal_cond(cond) calpriv_cat(calpriv_cond_, cond)
#define calpriv_cond_1(x, y) x
#define calpriv_cond_0(x, y) y

#if __STDC_VERSION__ >= 201112L
#define cal_static_assert _Static_assert
#else
#define cal_static_assert(cond, msg) \
    void cal_static_assert_dummy_function(char (*)[!!(cond) * 2 - 1])
#endif

#ifdef NDEBUG
#define cal_cond_ndebug(x, y) x
#else
/** Expand to the first argument if NDEBUG is defined.  Otherwise, expand to
    the second argument. */
#define cal_cond_ndebug(x, y) y
#endif

/** Obtain a lower bound on the logarithm of a number */
#define cal_minlog2(x)                          \
    ((x) >= 256 ? 8 :                           \
     (x) >= 128 ? 7 :                           \
     (x) >= 64 ? 6 :                            \
     (x) >= 32 ? 5 :                            \
     (x) >= 16 ? 4 :                            \
     (x) >= 8 ? 3 :                             \
     (x) >= 4 ? 2 :                             \
     (x) >= 2 ? 1 :                             \
     0)

/** A macro that expands to nothing.  Useful if you want to pass nothing to
    the argument of a function-like macro, since they are forbidden in older
    versions of C and C++. */
#define CAL_NOARG

/** @def CAL_WARNUNUSED
    If available, expand to a built-in function attribute that warns if the
    function is called without using its result. */
#ifdef __clang__
#define CAL_WARNUNUSED __attribute__((warn_unused_result))
#else
/* don't enable this on GCC
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425 */
#define CAL_WARNUNUSED
#endif

/* workaround for https://llvm.org/bugs/show_bug.cgi?id=22712 */
#ifdef __clang__
#define CAL_UNUSED __attribute__ ((unused))
#else
#define CAL_UNUSED
#endif

/** @def CAL_FUNC
    Expand to `__func__` if available, else `NULL`. */
#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
#define CAL_FUNC __func__
#else
#define CAL_FUNC NULL
#endif
