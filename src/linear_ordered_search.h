#ifndef G_IB1RSJRJ5ELTXW92F82R0LCRDDI15
#define G_IB1RSJRJ5ELTXW92F82R0LCRDDI15
/*@self.public()*/
/** @file
    Linear ordered search. */
#include <stddef.h>
#include "compat/inline_begin.h"

/** Perform a linear search on a pre-sorted array pointed by `ptr`.  If the
    search was successful, `1` is returned and `*pos_out` is set to the index
    of the element that compares equal to `key`.  Otherwise, `0` is returned
    and `*pos_out` is set to the index of the smallest element greater than
    `key`.  `pos_out` is optional and can be null. */
static inline
int cal_linear_ordered_search(const void *key,
                              const void *ptr,
                              size_t count,
                              size_t size,
                              int (*cmp)(void *, const void *, const void *),
                              void *cmp_ctx,
                              size_t *pos_out)
{
    int ret = 0;
    size_t i;
    for (i = 0; i < count; ++i) {
        int r = (*cmp)(cmp_ctx, key, (const char *)ptr + i * size);
        if (!r) {
            ret = 1;
        }
        if (r <= 0) {
            break;
        }
    }
    if (pos_out) {
        *pos_out = i;
    }
    return ret;
}

#include "compat/inline_end.h"
#endif
