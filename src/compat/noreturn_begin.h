#ifdef RF_NORETURN_DEFINED
# error "must not include begin header twice in succession"
#else
# ifndef noreturn
#  if __STDC_VERSION__ >= 201112L
#   define noreturn _Noreturn
#  elif __cplusplus >= 201103L
#   define noreturn [[noreturn]]
#  elif defined __GNUC__
#   define noreturn __attribute__((noreturn))
#  elif defined __MSC_VER
#   define noreturn __declspec(noreturn)
#  else
#   define noreturn
#  endif
#  define RF_NORETURN_DEFINED 1
# else
#  define RF_NORETURN_DEFINED 0
# endif
#endif
