#ifndef G_FU4GEW35ROFUW4GO3WDMZA7P80W0J
#define G_FU4GEW35ROFUW4GO3WDMZA7P80W0J
/** @file
    Shuffling algorithm. */
#include <stddef.h>
#include <string.h>

/** Shuffles elements in an array.

    @param array
    A pointer to an array of elements.

    @param count
    The number of elements in `array`.

    @param size
    The size of each element in bytes.

    @param buffer
    A pointer to a temporary writable buffer capable of holding at
    least `size` bytes.  It must not alias `array`.

    @param randfunc
    A function that generates a nonnegative random value less than or equal to
    its second argument.

    @param randfunc_ctx
    An arbitrary value passed as the first argument to `random`.

*/
static inline
void cal_shuffle(void *array,
                 size_t count,
                 size_t size,
                 void *buffer,
                 size_t (*randfunc)(void *, size_t),
                 void *randfunc_ctx)
{
    if (count) {
        size_t i;
        char *p = (char *)array;
        for (i = count - 1; i; --i) {
            size_t j = (*randfunc)(randfunc_ctx, i);
            memcpy(buffer, p + j * size, size);
            memmove(p + j * size, p + i * size, size);
            memcpy(p + i * size, buffer, size);
        }
    }
}

#endif
