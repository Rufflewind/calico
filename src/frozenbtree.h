#if 0

void rf_kst_build_subtree(Type *dest,
                          const Type *src,
                          size_t size,
                          size_t w, // related to dimofsubtree
                          size_t ancestry,
                          Type *levelptr,
                          size_t levelsize)
{
    static const size_t keyspernode = KeysPerNode;
    size_t dimofsubtree = w - 1;
    size_t maxremovablepersubtree, offset, remainder, numfilled, R;
    if (dimofsubtree) {
        maxremovablepersubtree = w / (keyspernode + 1) * keyspernode;
        // R = size - (k + 1) /* contribution from the pivots */
        //     - (k + 1) * (dimofsubtree - maxremovablepersubtree) /* the unremovable parts */;
        R = size + 1 - (keyspernode + 1) * (dimofsubtree - maxremovablepersubtree + 1);
        numfilled = R / maxremovablepersubtree;
        remainder = R % maxremovablepersubtree;
    }
    offset = 0;
    for (int i = 0; i != KeysPerNode + 1; ++i) {
        if (dimofsubtree) {
            Type *newlevelptr = levelptr + levelsize;
            size_t newlevelsize = levelsize * (keyspernode + 1);
            size_t newancestry = ancestry * (keyspernode + 1) + i;
            Type *newdest = newlevelptr + newancestry * keyspernode;
            const Type *newsrc = src + offset;
            size_t neww = w / (keyspernode + 1);
            if (numfilled) {
                size_t newsize = dimofsubtree;
                rf_kst_build_subtree(newdest, newsrc, newsize, neww,
                                     newancestry, newlevelptr, newlevelsize);
                offset += newsize;
                --numfilled;
            } else {
                size_t newsize = dimofsubtree - maxremovablepersubtree + remainder;
                rf_kst_build_subtree(newdest, newsrc, newsize, neww,
                                     newancestry, newlevelptr, newlevelsize);
                offset += newsize;
                remainder = 0;
            }
        }
        if (i != KeysPerNode) {
            dest[i] = src[offset];
            offset += 1;
        }
    }
}

void rf_kst_build(Type *dest, const Type *src, size_t size)
{
    static const size_t keyspernode = KeysPerNode;
    size_t w = keyspernode + 1;
    while (w - 1 < size) {
        w *= keyspernode + 1;
    }
    w /= keyspernode + 1;
    rf_kst_build_subtree(dest, src, size, w, 0, dest, keyspernode);
}

#endif

static
inline
int rf_btree_search(const Type *const key,
                    void **rootpptr,
                    size_t count,
                    size_t size,
                    void *const cmp_ctx,
                    size_t const keyspernode)
{
    size_t shift;
    size_t levelsize = keyspernode;
    size_t ancestry = 0;
    char *rootptr = (char *)*rootpptr;
    char *levelptr = (char *)rootptr;
    while (1) {
        const size_t span = keyspernode < count ? keyspernode : count;
        Type *lptr = rootptr;
        if (rf_bsearch_Type(key, &lptr, span, cmp_ctx)) {
            *prootptr = lptr;
            return 1;
        }
        ancestry = ancestry * (keyspernode + 1) + (lptr - rootptr) / size;
        shift = ancestry * keyspernode - (rootptr - levelptr) / size + levelsize;
        if (shift >= count) {
            *prootptr = UNKNOWN;
            return 0;
        }
        rootptr += shift * size;
        count -= shift;
        levelptr += levelsize * size;
        levelsize *= keyspernode + 1;
    }
}
