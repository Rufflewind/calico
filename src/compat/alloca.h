/*@#public*/
/** @file
    Compatibility header for `alloca`.

    This header provides an `alloca`-like function using variable-length
    arrays (VLAs), a built-in `alloca` function, or using `malloc`.

    Two functions are provided:

      - `cal_alloca(type, var, count)`
      - `cal_freea(var)`

    These functions should always be used like this:

        void my_function(size_t count)
        {
            cal_alloca(double, mybuf, count);
            // do something with mybuf ...
            cal_freea(mybuf);
        }

    Do not use `cal_alloca` anywhere except in the outermost scope of a
    function, as VLAs and alloca have different lifetimes.  Additionally,
    `cal_freea` must (and can only) be called right before the function
    returns.  Using `longjmp` with to jump out of any function that uses
    `cal_alloca` causes undefined behavior.

    This header can be included multiple times.

*/
#if __STDC_VERSION__ >= 199901L && !defined __STDC_NO_VLA__
#  define cal_alloca(type, var, count) \
     type _##var##_vla[count]; \
     type *var = _##var##_vla
#  define cal_freea(var)
#elif defined HAVE_ALLOCA
#  if defined HAVE_ALLOCA_H
#    include <alloca.h>
#  elif defined __GNUC__
#    define calpriv_alloca __builtin_alloca
#  elif defined _AIX
#    define calpriv_alloca __alloca
#  elif defined _MSC_VER
#    include <malloc.h>
#    define calpriv_alloca _alloca
#  endif
#  ifndef calpriv_alloca
#    define calpriv_alloca alloca
#  endif
#  define cal_alloca(type, var, count) \
     type *var = (type *)calpriv_alloca(count * sizeof(type))
#  define cal_freea(var)
#else
#  include <stdlib.h>
#  define cal_alloca(type, var, count) \
     type *var = (type *)malloc(count * sizeof(type))
#  define cal_freea(var) free(var)
#endif
