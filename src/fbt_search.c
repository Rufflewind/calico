#include <stddef.h>

#include "binary_search.h"

int fbt_search(const void *key,
               const void *ptr,
               size_t count,
               size_t size,
               int (*cmp)(void *, const void *, const void *),
               void *cmp_ctx,
               size_t keyspernode,
               size_t *pos_out)
{
    size_t levelsize = keyspernode;
    size_t ancestry = 0;
    size_t root = 0;
    size_t level = 0;
    size_t pos;
    while (!binary_search(key,
                          (const char *)ptr + root * size,
                          keyspernode < count ? keyspernode : count,
                          size,
                          cmp,
                          cmp_ctx,
                          &pos)) {
        size_t shift;
        ancestry = ancestry * (keyspernode + 1) + pos;
        level += levelsize;
        shift = ancestry * keyspernode - root + level;
        if (shift >= count) {
            return 0;
        }
        root += shift;
        count -= shift;
        levelsize *= keyspernode + 1;
    }
    *pos_out = root * size + pos;
    return 1;
}
