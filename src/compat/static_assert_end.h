#if !defined RF_STATIC_ASSERT_DEFINED
# error "must not include end header without matching begin"
#elif RF_STATIC_ASSERT_DEFINED
# undef static_assert
#endif
#undef RF_STATIC_ASSERT_DEFINED
