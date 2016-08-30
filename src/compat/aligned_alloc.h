#ifndef G_YNR6UPCK12QOKCDW0PUI5LIMXTPC4
#define G_YNR6UPCK12QOKCDW0PUI5LIMXTPC4
#if !(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
# error Requirement not met: _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
#endif
#include <stdlib.h>

/** An implementation of C11's `aligned_alloc` that makes use of
    `posix_memalign`.  If `size` is not a multiple of `alignment`, or
    `alignment` is not a power of two, then the behavior is
    implementation-defined. */
static
void *cal_aligned_alloc(size_t alignment, size_t size);

void *cal_aligned_alloc(size_t alignment, size_t size)
{
    void *ptr;
    /* POSIX forbids `alignment` to be smaller than `sizeof(void *)`, so we
       must insist that. */
    if (alignment < sizeof(void *)) {
        /* Since C requires all alignments to be a power of two and POSIX
           requires `alignment` be a multiple of `sizeof(void *)`, it follows
           that `sizeof(void *)` must be a power of two in POSIX, hence it's
           safe to assume that this is a valid and stricter alignment. */
        alignment = sizeof(void *);
    }
    if (posix_memalign(&ptr, alignment, size)) {
        return NULL;
    }
    return ptr;
}

#endif
