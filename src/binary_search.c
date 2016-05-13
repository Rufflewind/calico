#include <stddef.h>

/** Perform a binary search on a sorted array pointed by `ptr`.  If the search
    was successful, `1` is returned and `*pos_out` is set to the index of the
    element that compares equal to `key`.  Otherwise, `0` is returned and
    `*pos_out` is set to the index of the smallest element greater than `key`.
    `pos_out` is optional and can * be null. */
int binary_search(const void *key,
                  const void *ptr,
                  size_t count,
                  size_t size,
                  int (*cmp)(void *, const void *, const void *),
                  void *cmp_ctx,
                  size_t *pos_out)
{
    size_t p = 0;
    while (count) {
        size_t m = count / 2;
        char q = p + m;
        int r = (*cmp)(cmp_ctx, key, (const char *)ptr + q * size);
        if (!r) {
            if (pos_out) {
                *pos_out = q;
            }
            return 1;
        }
        if (r > 0) {
            m -= !(count % 2);
            p = q + 1;
        }
        count = m;
    }
    if (pos_out) {
        *pos_out = p;
    }
    return 0;
}
