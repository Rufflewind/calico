#include "macros.h"
#ifndef Type_Suffix
#define Type_Suffix Type()
#define RF_TYPE_SUFFIX_DEFINED
#endif
#define rf_cmp_Type rf_cat(rf_cmp_, Type_Suffix)
#ifdef __cplusplus
extern "C" {
#endif

static
int rf_cmp_Type(void *ctx, Type(const *left), Type(const *right));

#include "cmp_Type.inl"
#ifdef __cplusplus
}
#endif
#undef rf_cmp_Type
#ifdef RF_TYPE_SUFFIX_DEFINED
#undef RF_TYPE_SUFFIX_DEFINED
#undef Type_Suffix
#endif
#include "cmpg_Type.h"
