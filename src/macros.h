#ifndef G_WFFUK43TAWD9K37V4KKAHPQ6PEEXI
#define G_WFFUK43TAWD9K37V4KKAHPQ6PEEXI
/** @file

    Miscellaneous macros. */

/** Concatenate two tokens. */
#define rf_cat(x, y) rf_cat_(x, y)
#define rf_cat_(x, y) x ## y

/** Convert an expression into a string literal. */
#define rf_stringify(x) rf_stringify_(x)
#define rf_stringify_(x) #x

#define rf_min(x, y) ((x) < (y) ? (x) : (y))

#define rf_max(x, y) ((x) >= (y) ? (x) : (y))

#define rf_cmp(x, y) (((x) > (y)) - ((x) < (y)))

/* don't enable this for GCC
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425 */
#ifdef __clang__
# define RF_WARNUNUSED __attribute__((warn_unused_result))
#else
# define RF_WARNUNUSED
#endif

#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
# define RF_FUNC __func__
#else
# define RF_FUNC NULL
#endif

#endif
