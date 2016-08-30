/*@self.public()*/
/** @file
    Compatibility header for `aligned_alloc`. */
#if !defined CAL_ALIGNED_ALLOC_DEFINED
# error "must not include end header without matching begin"
#elif CAL_ALIGNED_ALLOC_DEFINED
# undef aligned_alloc
#endif
#undef CAL_ALIGNAS_DEFINED
