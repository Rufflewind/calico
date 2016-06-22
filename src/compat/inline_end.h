/*@self.public()*/
/** @file
    Compatibility header for `inline`. */
#if !defined CAL_INLINE_DEFINED
# error "must not include end header without matching begin"
#elif CAL_INLINE_DEFINED
# undef inline
#endif
#undef CAL_INLINE_DEFINED
