#include <stddef.h>

int binary_search(const void *key,
                  void **pptr,
                  size_t count,
                  size_t size,
                  int (*cmp)(void *, const void *, const void *),
                  void *cmp_ctx)
{
    char *p = (char *)*pptr;
    while (count) {
        size_t m = count / 2;
        char *q = p + m * size;
        int r = (*cmp)(cmp_ctx, key, q);
        if (!r) {
            *pptr = q;
            return 1;
        }
        if (r > 0) {
            m -= !(count % 2);
            p = q + size;
        }
        count = m;
    }
    *pptr = p;
    return 0;
}
