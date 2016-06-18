/*@#public*/
/** @file
    Compatibility header for `static_assert`. */
#if !defined CAL_STATIC_ASSERT_DEFINED
# error "must not include end header without matching begin"
#elif CAL_STATIC_ASSERT_DEFINED
# undef static_assert
#endif
#undef CAL_STATIC_ASSERT_DEFINED
