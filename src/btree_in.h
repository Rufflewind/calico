#ifndef G_6Z7W8UA9FODDEZVAJ1AST1BZP2MZU
#define G_6Z7W8UA9FODDEZVAJ1AST1BZP2MZU
#include "btree_head.h"

/**
   Hint: Use cal_pcmp for built-in types, and cal_pstrcmp for strings.
 */

#define cal_def_btree(Prefix, B, K, V, HasValue, CompareFunction, \
                      SearchFunction, ChildIndexType, HeightType, inline) \
/*@
escape_newlines(
    replace_ident(
        prepend_prefix(
            replace_ident(
                load_file("btree_impl.h"),
                "static_assert",
                "cal_static_assert"),
            public_pattern=r"btree(_.*)?",
            exclude_pattern=r"cal_.*",
            private_subprefix="priv_btree"),
        "inline",
        "inline CAL_UNUSED"))
*/

#define cal_def_btreemap(Prefix, K, V, CompareFunction) \
    cal_def_btree(Prefix, 16, K, V, 1, CompareFunction, \
                  linear_sorted_search, unsigned short, unsigned char, inline)

#define cal_def_btreeset(Prefix, K, CompareFunction) \
    cal_def_btree(Prefix, 16, K, void, 0, CompareFunction, \
                  linear_sorted_search, unsigned short, unsigned char, inline)

#endif
