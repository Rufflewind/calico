#ifdef RF_STATIC_ASSERT_DEFINED
# error "must not include begin header twice in succession"
#else
# if !defined static_assert && __cplusplus < 201103L
#  if __STDC_VERSION__ >= 20112L
#   define static_assert _Static_assert
#  else
#   define static_assert(cond, msg) \
    void rf_static_assert_dummy_function(char (*)[!!(cond) * 2 - 1])
#  endif
#  define RF_STATIC_ASSERT_DEFINED 1
# else
#  define RF_STATIC_ASSERT_DEFINED 0
# endif
#endif
