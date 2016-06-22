/*@self.public()*/
/** @file
    Compatibility header for `alignas`. */
#if !defined CAL_ALIGNAS_DEFINED
# error "must not include end header without matching begin"
#elif CAL_ALIGNAS_DEFINED
# undef alignas
#endif
#undef CAL_ALIGNAS_DEFINED
