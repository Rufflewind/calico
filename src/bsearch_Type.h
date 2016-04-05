#include "macros.h"
#ifndef Type_Suffix
#define Type_Suffix Type()
#define RF_TYPE_SUFFIX_DEFINED
#endif
#define rf_cmpg_Type rf_cat(rf_cmpg_, Type_Suffix)
#define rf_bsearch_Type rf_cat(rf_bsearch_, Type_Suffix)
#define rf_bsearchc_Type rf_cat(rf_bsearchc_, Type_Suffix)
#ifdef __cplusplus
extern "C" {
#endif

static
int rf_bsearch_Type(Type(const *key),
                    Type(**pptr),
                    size_t count,
                    void *cmp_ctx);

static
int rf_bsearchc_Type(Type(const *key),
                     Type(const **pptr),
                     size_t count,
                     void *cmp_ctx);

#include "bsearch_Type.inl"
#ifdef __cplusplus
}
#endif
#undef rf_cmpg_Type
#undef rf_bsearch_Type
#undef rf_bsearchc_Type
#ifdef RF_TYPE_SUFFIX_DEFINED
#undef RF_TYPE_SUFFIX_DEFINED
#undef Type_Suffix
#endif
