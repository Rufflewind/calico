#include "compat/inline_begin.h"

inline
int rf_cmp_Type(void *ctx, Type(const *left), Type(const *right))
{
    (void)ctx;
    return rf_cmp(*left, *right);
}

#include "compat/inline_end.h"
