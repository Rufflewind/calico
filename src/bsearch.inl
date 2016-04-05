#include "compat/inline_begin.h"

inline
int rf_bsearch(const void *const key,
               void **const pptr,
               size_t count,
               size_t const size,
               int (*const cmp)(void *, const void *, const void *),
               void *const cmp_ctx)
{
    char *ptr = (char *)*pptr;
    while (count) {
        size_t mid = count / 2;
        char *const midptr = ptr + mid * size;
        int const rcmp = (*cmp)(cmp_ctx, key, midptr);
        if (!rcmp) {
            *pptr = midptr;
            return 1;
        }
        if (rcmp > 0) {
            mid -= !(count % 2);
            ptr = midptr + size;
        }
        count = mid;
    }
    *pptr = ptr;
    return 0;
}

#include "compat/inline_end.h"
