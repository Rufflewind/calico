import utils

m = utils.TemplateMaker(
    impl_filename="btree_impl.h",
    public_pattern=r"btree(_.*)?",
    exclude_pattern=r"cal_.*|static_assert",
    private_subprefix="priv_btree",
    params=[
        "KeyType",
        "ValueType=void",
        "CompareFunction=cal_pcmp",
        "SearchFunction=cal_linear_ordered_search",
        "MinArity=8",
        "ChildIndexType=unsigned short",
        "HeightType=unsigned char",
    ],
)
__deps__ = m.deps
main = m.main
