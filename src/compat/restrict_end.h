/*@#public*/
/** @file
    Compatibility header for `restrict`. */
#if !defined CAL_RESTRICT_DEFINED
# error "must not include end header without matching begin"
#elif CAL_RESTRICT_DEFINED
# undef restrict
#endif
#undef CAL_RESTRICT_DEFINED
