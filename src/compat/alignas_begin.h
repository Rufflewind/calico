#ifdef CAL_ALIGNAS_DEFINED
# error "must not include begin header twice in succession"
#else
# if !defined alignas && __cplusplus < 201103L
#  if __STDC_VERSION__ >= 201112L
#   define alignas _Alignas
#   define CAL_ALIGNAS_DEFINED 1
#  elif defined __GNUC__
#   define alignas(x) __attribute__((aligned(x)))
#   define CAL_ALIGNAS_DEFINED 1
#  elif defined _MSC_VER
#   define alignas(x) __declspec(align(x))
#   define CAL_ALIGNAS_DEFINED 1
#  elif defined CAL_ALIGNAS_OPTIONAL
#   define alignas(x)
#   define CAL_ALIGNAS_DEFINED 1
#  else
#   error "C++11 or C11 is required for alignas specifier"
#  endif
# endif
# ifndef CAL_ALIGNAS_DEFINED
#  define CAL_ALIGNAS_DEFINED 0
# endif
#endif
