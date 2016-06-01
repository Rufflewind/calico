#include "compat/inline_begin.h"
#include "compat/static_assert_begin.h"
#ifndef HasValue
# ifdef V
#  define HasValue 1
# else
#  define HasValue 0
# endif
#endif
/*@
make_template(
    "btree_impl.h",
    public_pattern=r"btree(_.*)?",
    exclude_pattern=r"cal_.*|static_assert",
    private_subprefix="priv_btree",
    params=[
        "B=8",
        "K",
        "V=void",
        "CompareFunction=cal_pcmp",
        "SearchFunction=linear_sorted_search",
        "ChildIndexType=unsigned short",
        "HeightType=unsigned char",
    ],
)
*/
#undef HasValue
#include "compat/inline_end.h"
#include "compat/static_assert_end.h"
