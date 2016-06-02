#ifndef G_WZJZCCUAAKWCCDXAXI6O6GC574KDM
#define G_WZJZCCUAAKWCCDXAXI6O6GC574KDM
#include <stddef.h>
#include "compat/inline_begin.h"

/** Perform a binary search on a sorted array pointed by `ptr`.  If the search
    was successful, `1` is returned and `*pos_out` is set to the index of the
    element that compares equal to `key`.  Otherwise, `0` is returned and
    `*pos_out` is set to the index of the smallest element greater than `key`.
    `pos_out` is optional and can be null. */
static inline
int binary_search(const void *key,
                  const void *ptr,
                  size_t count,
                  size_t size,
                  int (*cmp)(void *, const void *, const void *),
                  void *cmp_ctx,
                  size_t *pos_out)
{
    size_t i = 0;
    while (count) {
        size_t m = count / 2;
        size_t j = i + m;
        int r = (*cmp)(cmp_ctx, key, (const char *)ptr + j * size);
        if (!r) {
            if (pos_out) {
                *pos_out = j;
            }
            return 1;
        }
        if (r > 0) {
            m -= !(count % 2);
            i = j + 1;
        }
        count = m;
    }
    if (pos_out) {
        *pos_out = i;
    }
    return 0;
}

#include "compat/inline_end.h"
#endif