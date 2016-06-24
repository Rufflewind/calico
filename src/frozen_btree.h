#ifndef G_YN2V4ZB84SBE8WCYQNYU9Q9A3WVA3
#define G_YN2V4ZB84SBE8WCYQNYU9Q9A3WVA3
/** @file
    Frozen B-tree.
*/
#include <assert.h>
#include <string.h>
#include "binary_search.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "compat/inline_begin.h"

static inline
void calpriv_fbt_build_subtree(void *dest,
                               const void *src,
                               size_t count,
                               size_t size,
                               size_t children_per_node,
                               size_t ancestry,
                               size_t level_count,
                               size_t dim)
{
    char *cdest = (char *)dest;
    const char *csrc = (const char *)src;
    size_t keys_per_node = children_per_node - 1;
    size_t subdim = dim / children_per_node;
    size_t excess = count + 1 - subdim;
    size_t subsubdim = subdim / children_per_node;
    size_t min_subcount = subsubdim - 1;
    size_t max_basecount = subdim - subsubdim;
    size_t i, pos = 0;
    if (count <= keys_per_node) {
        memcpy(cdest + ancestry * keys_per_node * size, csrc, count * size);
        return;
    }
    for (i = 0; i < children_per_node; ++i) {
        size_t basecount = max_basecount < excess ? max_basecount : excess;
        size_t subcount = min_subcount + basecount;
        size_t subancestry = ancestry * children_per_node + i;
        excess -= basecount;
        if (i) {
            memcpy(cdest + (ancestry * keys_per_node + i - 1) * size,
                   csrc + pos * size,
                   size);
            ++pos;
        }
        calpriv_fbt_build_subtree(cdest + level_count * size,
                                  csrc + pos * size,
                                  subcount,
                                  size,
                                  children_per_node,
                                  subancestry,
                                  level_count * children_per_node,
                                  subdim);
        pos += subcount;
    }
}

/** Create a frozen B-tree from a sorted array. */
static inline
void cal_fbt_build(void *dest,
                   const void *src,
                   size_t count,
                   size_t size,
                   size_t children_per_node)
{
    /* Let B = number of children by per node.
       Then, in a tree of height H, there are
       (pow(B, H) - 1) total elements, and
       ((B - 1) * pow(B, H - 1)) elements in the base level. */
    size_t dim = 1;
    while (dim <= count) {
        dim *= children_per_node;
    }
    calpriv_fbt_build_subtree(dest,
                              src,
                              count,
                              size,
                              children_per_node,
                              0,
                              children_per_node - 1,
                              dim);
}

/** Search a frozen B-tree, with the inner search performed using the given
    function. */
static inline
int cal_fbt_search_generic(const void *key,
                           const void *ptr,
                           size_t count,
                           size_t size,
                           int (*cmp)(void *, const void *, const void *),
                           void *cmp_ctx,
                           int (*search)(void *,
                                         const void *,
                                         const void *,
                                         size_t,
                                         size_t,
                                         int (*)(void *,
                                                 const void *,
                                                 const void *),
                                         void *,
                                         size_t *),
                           void *search_ctx,
                           size_t children_per_node,
                           size_t *pos_out)
{
    int found;
    size_t keys_per_node = children_per_node - 1;
    size_t level_count = keys_per_node, ancestry = 0, level_pos = 0, pos = 0;
    assert(children_per_node > 1);
    while (1) {
        size_t i, remaining = count - pos;
        size_t span = keys_per_node < remaining ? keys_per_node : remaining;
        found = (*search)(search_ctx, key, (const char *)ptr + pos * size,
                          span, size, cmp, cmp_ctx, &i);
        if (found) {
            *pos_out = pos + i;
            break;
        }
        ancestry = ancestry * children_per_node + i;
        pos = level_pos + level_count + ancestry * keys_per_node;
        if (pos >= count) {
            break;
        }
        level_pos += level_count;
        level_count *= children_per_node;
    }
    return found;
}

static inline
int calpriv_fbt_search_b_search(void *ctx,
                                const void *key,
                                const void *ptr,
                                size_t count,
                                size_t size,
                                int (*cmp)(void *,
                                           const void *,
                                           const void *),
                                void *cmp_ctx,
                                size_t *pos_out)
{
    (void)ctx;
    return cal_binary_search(key, ptr, count, size, cmp, cmp_ctx, pos_out);
}

/** Search a frozen B-tree, with the inner search performed using binary
    search. */
static inline
int cal_fbt_search_b(const void *key,
                     const void *ptr,
                     size_t count,
                     size_t size,
                     int (*cmp)(void *, const void *, const void *),
                     void *cmp_ctx,
                     size_t children_per_node,
                     size_t *pos_out)
{
    return cal_fbt_search_generic(key,
                                  ptr,
                                  count,
                                  size,
                                  cmp,
                                  cmp_ctx,
                                  &calpriv_fbt_search_b_search,
                                  NULL,
                                  children_per_node,
                                  pos_out);
}

static inline
int calpriv_fbt_search_l_search(void *ctx,
                                const void *key,
                                const void *ptr,
                                size_t count,
                                size_t size,
                                int (*cmp)(void *,
                                           const void *,
                                           const void *),
                                void *cmp_ctx,
                                size_t *pos_out)
{
    (void)ctx;
    return cal_binary_search(key, ptr, count, size, cmp, cmp_ctx, pos_out);
}

/** Search a frozen B-tree, with the inner search performed using linear
    search. */
static inline
int cal_fbt_search_l(const void *key,
                     const void *ptr,
                     size_t count,
                     size_t size,
                     int (*cmp)(void *, const void *, const void *),
                     void *cmp_ctx,
                     size_t children_per_node,
                     size_t *pos_out)
{
    return cal_fbt_search_generic(key,
                                  ptr,
                                  count,
                                  size,
                                  cmp,
                                  cmp_ctx,
                                  &calpriv_fbt_search_l_search,
                                  NULL,
                                  children_per_node,
                                  pos_out);
}

#include "compat/inline_end.h"
#ifdef __cplusplus
}
#endif
#endif
