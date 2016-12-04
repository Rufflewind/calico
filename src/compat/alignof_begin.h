/*@self.public()*/
/** @file
    Compatibility header for `alignof`.

    Be sure to use simple type expressions that work with the `TYPE VARIABLE;`
    declaration syntax.  Otherwise, the fallback methods may not work.  Use
    `typedef` if necessary to simplify the type expression.

*/
#ifdef CAL_ALIGNOF_DEFINED
# error "must not include begin header twice in succession"
#else
# if !defined alignof && __cplusplus < 201103L
#  if __STDC_VERSION__ >= 199901L
#   define alignof _Alignof
#   define CAL_ALIGNOF_DEFINED 1
#  elif defined __GNUC__
#   define alignof __alignof__
#   define CAL_ALIGNOF_DEFINED 1
#  elif defined _MSC_VER
#   define alignof __alignof
#   define CAL_ALIGNOF_DEFINED 1
#  else
#   include <stddef.h>
#   define alignof(x) offsetof(struct { char c; x m; }, m)
#   define CAL_ALIGNOF_DEFINED 1
#  endif
# endif
# ifndef CAL_ALIGNOF_DEFINED
#  define CAL_ALIGNOF_DEFINED 0
# endif
#endif
