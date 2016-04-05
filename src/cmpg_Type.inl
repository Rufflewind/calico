#include "compat/inline_begin.h"

inline
int rf_cmpg_Type(void *ctx, const void *left, const void *right)
{
    return rf_cmp_Type(ctx, (Type(const *))left, (Type(const *))right);
}

#include "compat/inline_end.h"
