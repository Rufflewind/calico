#ifndef G_FU4GEW35ROFUW4GO3WDMZA7P80W0J
#define G_FU4GEW35ROFUW4GO3WDMZA7P80W0J
#include <stddef.h>
#include <string.h>

/** Shuffles elements in the array pointed by `array` containing `count`
    elements of the given `size`.  The temporary buffer `buf` must be able to
    hold up at least one element. */
static inline
void cal_shuffle(void *array, size_t count, size_t size, void *buf)
{
    if (count) {
        size_t i;
        char *p = (char *)array;
        for (i = count - 1; i; --i) {
            size_t j = (size_t)rand() % (i + 1);
            memcpy(buf, p + j * size, size);
            memmove(p + j * size, p + i * size, size);
            memcpy(p + i * size, buf, size);
        }
    }
}

#endif
