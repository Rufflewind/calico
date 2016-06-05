#include "compat/inline_begin.h"
#include "compat/static_assert_begin.h"
#ifndef HasValue
#ifdef ValueType
#define HasValue 1
#else
#define HasValue 0
#endif
#endif
/*@
make_template(
    "btree_impl.h",
    public_pattern=r"btree(_.*)?",
    exclude_pattern=r"cal_.*|static_assert",
    private_subprefix="priv_btree",
    params=[
        "KeyType",
        "ValueType=void",
        "CompareFunction=cal_pcmp",
        "SearchFunction=linear_ordered_search",
        "MinArity=8",
        "ChildIndexType=unsigned short",
        "HeightType=unsigned char",
    ],
)
*/
#undef HasValue
#include "compat/inline_end.h"
#include "compat/static_assert_end.h"
