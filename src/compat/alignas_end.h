#if !defined RF_ALIGNAS_DEFINED
# error "must not include end header without matching begin"
#elif RF_ALIGNAS_DEFINED
# undef alignas
#endif
#undef RF_ALIGNAS_DEFINED
