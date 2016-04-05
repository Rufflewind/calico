#include "compat/inline_begin.h"

inline
int rf_bsearch_Type(Type(const *key),
                    Type(**pptr),
                    size_t count,
                    void *cmp_ctx)
{
    void *ptr = *pptr;
    int const r = rf_bsearch(key, &ptr, count, sizeof(Type()),
                             &rf_cmpg_Type, cmp_ctx);
    *pptr = (Type(*))ptr;
    return r;
}

inline
int rf_bsearchc_Type(Type(const *key),
                     Type(const **pptr),
                     size_t count,
                     void *cmp_ctx)
{
    Type(*ptr) = (Type(*))*pptr;
    int const r = rf_bsearch_Type(key, &ptr, count, cmp_ctx);
    *pptr = (const Type(*))ptr;
    return r;
}

#include "compat/inline_end.h"
