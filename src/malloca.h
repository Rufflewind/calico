#ifndef MALLOCA

/* Note: do not use MALLOCA anywhere except in the outermost scope of a
   function, as VLAs and alloca have different lifetimes */

#if __STDC_VERSION__ >= 199901L && !defined __STDC_NO_VLA__
#  define MALLOCA(type, var, count) \
     type _##var##_vla[count]; \
     type *var = _##var##_vla
#  define FREEA(var)
#elif defined HAVE_ALLOCA
#  if defined HAVE_ALLOCA_H
#    include <alloca.h>
#  elif defined __GNUC__
#    define alloca __builtin_alloca
#  elif defined _AIX
#    define alloca __alloca
#  elif defined _MSC_VER
#    include <malloc.h>
#    define alloca _alloca
#  endif
#  define MALLOCA(type, var, count) \
     type *var = (type *)alloca(count * sizeof(type))
#  define FREEA(var)
#else
#  define MALLOCA(type, var, count) \
     type *var = (type *)malloc(count * sizeof(type))
#  define FREEA(var) free(var)
#endif

#endif
