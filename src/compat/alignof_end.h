/*@self.public()*/
/** @file
    Compatibility header for `alignof`. */
#if !defined CAL_ALIGNOF_DEFINED
# error "must not include end header without matching begin"
#elif CAL_ALIGNOF_DEFINED
# undef alignof
#endif
#undef CAL_ALIGNOF_DEFINED
