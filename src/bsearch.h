#ifndef G_I0D4JCCXC8BQYQO4MGO8R1LD5AMHK
#define G_I0D4JCCXC8BQYQO4MGO8R1LD5AMHK
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/** Perform a binary search on an array `*pptr` to locate `*key`.  The array
    contains `count` elements, each of `size` bytes.

    Comparison is performed using the `*cmp` function, with the opaque
    argument `cmp_ctx` supplied as its first argument.  The function should
    return a negative value if `*left` is less than `*right`, a positive value
    if greater than, and zero if equal.  The function must not modify `*left`
    nor `*right`.  It is assumed that `*cmp` forms a total ordering on the
    elements.

    If the element is found, a nonzero value is returned.  Otherwise, zero is
    returned.  The pointer to least value that is not less than `*key` is
    stored in `*pptr`.

    The array itself is not modified during the process.

*/
static
int rf_bsearch(const void *key,
               void **pptr,
               size_t count,
               size_t size,
               int (*cmp)(void *ctx, const void *left, const void *right),
               void *cmp_ctx);

#include "bsearch.inl"
#ifdef __cplusplus
}
#endif
#endif
