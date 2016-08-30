/*@self.public()*/
/** @file
    Compatibility header for `aligned_alloc`. */
#if __STDC_VERSION__ >= 201112L || _ISOC11_SOURCE || HAVE_ALIGNED_ALLOC
# include <stdlib.h>
# define CAL_ALIGNED_ALLOC_DEFINED 0
#elif defined _MSC_VER
# include <malloc.h>
# define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)
# define CAL_ALIGNED_ALLOC_DEFINED 1
#else
# include "aligned_alloc.h"
# define aligned_alloc(alignment, size) cal_aligned_alloc(alignment, size)
# define CAL_ALIGNED_ALLOC_DEFINED 1
#endif
