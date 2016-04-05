#include "macros.h"
#ifndef Type_Suffix
#define Type_Suffix Type()
#define RF_TYPE_SUFFIX_DEFINED
#endif
#define rf_cmp_Type rf_cat(rf_cmp_, Type_Suffix)
#define rf_cmpg_Type rf_cat(rf_cmpg_, Type_Suffix)
#ifdef __cplusplus
extern "C" {
#endif

static
int rf_cmpg_Type(void *ctx, const void *left, const void *right);

#include "cmpg_Type.inl"
#ifdef __cplusplus
}
#endif
#undef rf_cmp_Type
#undef rf_cmpg_Type
#ifdef RF_TYPE_SUFFIX_DEFINED
#undef RF_TYPE_SUFFIX_DEFINED
#undef Type_Suffix
#endif
