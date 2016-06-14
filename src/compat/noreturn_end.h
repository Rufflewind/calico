#if !defined CAL_NORETURN_DEFINED
# error "must not include end header without matching begin"
#elif CAL_NORETURN_DEFINED
# undef noreturn
#endif
#undef CAL_NORETURN_DEFINED
