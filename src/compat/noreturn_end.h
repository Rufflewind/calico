#if !defined RF_NORETURN_DEFINED
# error "must not include end header without matching begin"
#elif RF_NORETURN_DEFINED
# undef noreturn
#endif
#undef RF_NORETURN_DEFINED
