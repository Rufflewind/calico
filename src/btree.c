#include <stddef.h>
#define B 2
#define K size_t
#define V double
static inline
int compare_K(const K *x, const K *y)
{
    return (*x > *y) - (*x < *y);
}

#include <assert.h>
#include <stdlib.h>
#include <stdio.h> /* for printing error messages */
#include <string.h>

/** Perform a linear search on a sorted array pointed by `ptr`.  If the search
    was successful, `1` is returned and `*pos_out` is set to the index of the
    element that compares equal to `key`.  Otherwise, `0` is returned and
    `*pos_out` is set to the index of the smallest element greater than
    `key`. */
static inline
int linear_sorted_search(const K *key,
                         const K *ptr,
                         size_t count,
                         size_t *pos_out)
{
    int ret = 0;
    size_t i = 0;
    for (i = 0; i < count; ++i) {
        int r = compare_K(key, ptr + i);
        if (!r) {
            ret = 1;
        }
        if (r <= 0) {
            break;
        }
    }
    *pos_out = i;
    return ret;
}

typedef struct {
    /* the number of valid keys (or number of valid values),
       ranging from 0 to 2 * B - 1 */
    size_t _len;
    /* an array of keys, with [0, _len) valid */
    K _keys[2 * B - 1];
#ifdef V
    /* an array of values, with [0, _len) valid */
    V _values[2 * B - 1];
#endif
} leaf_node;

static inline
size_t *leaf_len(leaf_node *m)
{
    return &m->_len;
}

static inline
K *leaf_keys(leaf_node *m)
{
    return m->_keys;
}

static inline
V *leaf_values(leaf_node *m)
{
    return m->_values;
}

typedef struct branch_node_ {
    /* we use `leaf_node` as a "base type" */
    leaf_node _data;
    /* child nodes, with [0, _data._len] valid */
    leaf_node *_children[2 * B];
} branch_node;

static inline
leaf_node **branch_children(branch_node *m)
{
    return m->_children;
}

static inline
leaf_node *branch_as_leaf(branch_node *m)
{
    return &m->_data;
}

/** May return NULL if it's not a branch. */
static inline
branch_node *try_leaf_as_branch(size_t height, leaf_node *m)
{
    if (!height) {
        return NULL;
    }
    return (branch_node *)m;
}

typedef struct {
    size_t _len;
    size_t _height;
    leaf_node *_root;
} btree;

void init_btree(btree *m)
{
    m->_len = 0;
    m->_height = 0;
    m->_root = NULL;
}

static
void free_node(size_t height, leaf_node *m)
{
    branch_node *mb = try_leaf_as_branch(height, m);
    if (mb) {
        for (size_t i = 0; i < *leaf_len(m) + 1; ++i) {
            free_node(height - 1, branch_children(mb)[i]);
        }
    }
    // printf("free(%p)\n", (void *)m);
    free(m);
}

void reset_btree(btree *m)
{
    free_node(m->_height - 1, m->_root);
    m->_len = 0;
    m->_height = 0;
    m->_root = NULL;
}

size_t btree_len(const btree *m)
{
    return m->_len;
}

/** Return the node and the position within that node. */
leaf_node *find_node(size_t height,
                     leaf_node *node,
                     const K *key,
                     size_t *index)
{
    size_t i;
    if (linear_sorted_search(key, leaf_keys(node), *leaf_len(node), &i)) {
        *index = i;
        return node;
    }
    branch_node *nodeb = try_leaf_as_branch(height, node);
    if (!nodeb) {
        return NULL;
    }
    --height;
    return find_node(height, branch_children(nodeb)[i], key, index);
}

/** Return the node and the position within that node. */
leaf_node *btree_get_node(btree *m, const K *key, size_t *index)
{
    if (!m->_root) {
        assert(m->_height == 0);
        assert(m->_len == 0);
        return NULL;
    }
    return find_node(m->_height, m->_root, key, index);
}

#ifdef V

V *btree_get(btree *m, const K *k)
{
    size_t i;
    leaf_node *n = btree_get_node(m, k, &i);
    if (!n) {
        return NULL;
    }
    return &leaf_values(n)[i];
}

const V *btree_get_const(const btree *m, const K *k)
{
    return btree_get((btree *)m, k);
}

#endif

int btree_in(const btree *m, const K *k)
{
    size_t i;
    return !!btree_get_node((btree *)m, k, &i);
}

int insert_node_here(size_t height,
                     leaf_node *node,
                     size_t i,
                     const K *key,
                     const V *value,
                     K *key_out,
                     V *value_out,
                     leaf_node **child_inout)
{

/* note that the we copy to the RIGHT child, hence the "+ 1" */
#define X_COPY(dstnode, srcnode, stmt)                                  \
    {                                                                   \
        K *dst = leaf_keys(dstnode);                                    \
        K *src = leaf_keys(srcnode);                                    \
        stmt;                                                           \
    }                                                                   \
    {                                                                   \
        V *dst = leaf_values(dstnode);                                  \
        V *src = leaf_values(srcnode);                                  \
        stmt;                                                           \
    }                                                                   \
    if (height) {                                                       \
        leaf_node **dst = branch_children((branch_node *)dstnode) + 1;  \
        leaf_node **src = branch_children((branch_node *)srcnode) + 1;  \
        stmt;                                                           \
    }

/* note that the we copy to the RIGHT child, hence the "+ 1" */
#define X_GET(node, stmt)                                               \
    {                                                                   \
        K *dst = leaf_keys(node);                                       \
        const K *src = key;                                             \
        stmt;                                                           \
    }                                                                   \
    {                                                                   \
        V *dst = leaf_values(node);                                     \
        const V *src = value;                                           \
        stmt;                                                           \
    }                                                                   \
    if (height) {                                                       \
        leaf_node **dst = branch_children((branch_node *)node) + 1;     \
        leaf_node **src = child_inout;                                  \
        stmt;                                                           \
    }

    size_t len = *leaf_len(node);

    /* case A: enough room to do a simple insert */
    if (len < 2 * B - 1) {
        X_COPY(node, node, {
            memmove(dst + i + 1, src + i, (len - i) * sizeof(*src));
        });
        X_GET(node, {
            dst[i] = *src;
        });
        ++*leaf_len(node);
        return 0;
    }

    /* case B: not enough room; need to split node */
    assert(len == 2 * B - 1);
    leaf_node *newnode = (leaf_node *)malloc(
        height ? sizeof(branch_node) : sizeof(leaf_node));
    if (!newnode) {
        /* FIXME: we should return 1 instead of failing, but we'd need to
           rollback the incomplete insert, which is tricky :c */
        fprintf(stderr, "%s:%lu: Out of memory\n",
                __FILE__, (unsigned long)__LINE__);
        fflush(stderr);
        abort();
    }
    size_t s = i > B ? i : B;
    X_COPY(newnode, node, {
        memcpy(dst + s - B, src + s, (B * 2 - 1 - s) * sizeof(*src));
    });
    if (i == B) {
        if (height) {
            branch_children((branch_node *)newnode)[0] = *child_inout;
        }
        *key_out = *key;
        *value_out = *value;
    } else {
        size_t mid = i < B ? B - 1 : B;
        K midkey = leaf_keys(node)[mid];;
        V midvalue = leaf_values(node)[mid];
        if (height) {
            branch_children((branch_node *)newnode)[0] =
                branch_children((branch_node *)node)[mid + 1];
        }
        if (i < B) {
            X_COPY(node, node, {
                memmove(dst + i + 1, src + i, (B - 1 - i) * sizeof(*src));
            });
            X_GET(node, {
                dst[i] = *src;
            });
        } else {
            X_COPY(newnode, node, {
                memcpy(dst, src + B + i, (i - B - 1) * sizeof(*src));
            });
            X_GET(newnode, {
                dst[i - B - 1] = *src;
            });
        }
        *key_out = midkey;
        *value_out = midvalue;
    }
    *leaf_len(node) = B;
    *leaf_len(newnode) = B - 1;
    *child_inout = newnode;
    return -2;

#undef X_ASSIGN
#undef X_COPY

}

/** Return the node and the position within that node. */
int insert_node(size_t height,
                leaf_node *node,
                const K *key,
                const V *value,
                K *key_inout,
                V *value_inout,
                leaf_node **child_inout)
{
    size_t i;
    /* this part depends on the comparison operation */
    if (linear_sorted_search(key, leaf_keys(node), *leaf_len(node), &i)) {
        /* exact match: just replace the value */
        leaf_values(node)[i] = *value;
        return -1;
    }
    /* the rest of it does not depend on the comparison operation, only on the
       layout of the structure */
    branch_node *nodeb = try_leaf_as_branch(height, node);
    if (nodeb) {
        int r = insert_node(height - 1,
                            branch_children(nodeb)[i],
                            key,
                            value,
                            key_inout,
                            value_inout,
                            child_inout);
        if (r >= -1) {                  /* we don't need to do any more */
            return r;
        }
    }
    return insert_node_here(height,
                            node,
                            i,
                            nodeb ? key_inout : key,
                            nodeb ? value_inout : value,
                            key_inout,
                            value_inout,
                            child_inout);
}

int btree_insert(btree *m, const K *key, const V *value)
{
    if (!m->_root) {
        assert(m->_len == 0);
        assert(m->_height == 0);
        m->_root = (leaf_node *)malloc(sizeof(*m->_root));
        if (!m->_root) {
            return 1;
        }
        ++m->_len; // uhhh wait what if the insert was a dupe
        m->_height = 1;
        *leaf_len(m->_root) = 1;
        leaf_keys(m->_root)[0] = *key;
        leaf_values(m->_root)[0] = *value;
        return 0;
    }
    K newkey;
    V newvalue;
    leaf_node *newchild;
    int r = insert_node(m->_height - 1,
                        m->_root,
                        key,
                        value,
                        &newkey,
                        &newvalue,
                        &newchild);
    if (r == 0) {                       /* added a new element */
        ++m->_len;
    } else if (r == -1) {               /* updated an existing element */
        r = 0;
    }
    if (r >= 0) {
        return r;
    }
    branch_node *newroot = (branch_node *)malloc(sizeof(*newroot));
    if (!newroot) {
        /* FIXME: we should return 1 instead of failing, but we'd need
           to rollback the incomplete insert, which is tricky :c */
        fprintf(stderr, "%s:%lu: Out of memory\n",
                __FILE__, (unsigned long)__LINE__);
        fflush(stderr);
        abort();
    }
    ++m->_height;
    *leaf_len(branch_as_leaf(newroot)) = 1;
    leaf_keys(branch_as_leaf(newroot))[0] = newkey;
    leaf_values(branch_as_leaf(newroot))[0] = newvalue;
    branch_children(newroot)[0] = m->_root;
    // printf("btree_insert: newchild=%p\n", (void *)newchild);
    branch_children(newroot)[1] = newchild;
    m->_root = branch_as_leaf(newroot);
    return 0;
}

#define INDENT 2

static
void dump_node(size_t indent, size_t height, leaf_node *m)
{
    // printf("dump_node(%p)\n", (void *)m);
    branch_node *mb = try_leaf_as_branch(height, m);
    size_t i;
    for (i = 0; i < *leaf_len(m); ++i) {
        if (mb) {
            dump_node(indent + INDENT, height - 1, branch_children(mb)[i]);
        }
        for (size_t j = 0; j < indent; ++j) {
            printf(" ");
        }
        if (height)
            printf("\033[37m%03zu\033[32m,%03.0f\033[0m\n",
                   leaf_keys(m)[i], leaf_values(m)[i]);
        else
            printf("\033[37m%03zu\033[33m,%03.0f\033[0m\n",
                   leaf_keys(m)[i], leaf_values(m)[i]);
    }
    if (mb) {
        dump_node(indent + INDENT, height - 1, branch_children(mb)[i]);
    }
}

/** For debugging purposes. */
void dump_btree(btree *m)
{
    if (!m->_root) {
        printf("(empty)\n");
    } else {
        dump_node(0, m->_height - 1, m->_root);
    }
    printf("----------------------------------------\n");
}

#include "wclock.h"
static wclock clk;
static int timing_counter;
static double clk_time;
#ifdef PROFILE
#define TIME(name)                                                      \
    for (clk_time = get_wclock(&clk), timing_counter = 0;               \
         !timing_counter;                                               \
         ++timing_counter,                                              \
         printf("time_%s=%.6g\n", name, get_wclock(&clk) - clk_time))
#else
#define TIME(name)
#endif

static
void test_random_inserts(unsigned seed,
                         unsigned range,
                         unsigned count,
                         int dump)
{
    btree bt, *t = &bt;
    init_btree(t);
    srand(seed);
    TIME("random_inserts1") {
        for (unsigned i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            double v = (double)((unsigned)rand() % range);
#ifndef PROFILE
            if (dump) {
                printf("insert(%zu, %f)\n", k, v);
            }
#endif
            assert(!btree_insert(t, &k, &v));
#ifndef PROFILE
            if (dump) {
                dump_btree(t);
            }
            assert(*btree_get(t, &k) == v);
#endif
        }
    }
    reset_btree(t);
}

int main(void)
{
    btree bt, *t = &bt;
    size_t k;
    double v;
    init_wclock(&clk);

    init_btree(t);
    reset_btree(t);

    // test_random_inserts(80, 10000000, 100000, 0);

    test_random_inserts(100, 100, 300, 1);
    // test_random_inserts(101, 100, 300, 1);
    // test_random_inserts(105, 100, 300, 1);
    // test_random_inserts(25, 25, 25, 1);

    // k = 0;
    // assert(!btree_get(t, &k));

    // k = 0;
    // v = 1;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 0;
    // assert(*btree_get(t, &k) == 1);

    // k = 0;
    // v = 2;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 0;
    // assert(*btree_get(t, &k) == 2);

    // k = 9;
    // v = 3;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 5;
    // v = 4;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 3;
    // v = 5;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 4;
    // v = 8;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 1;
    // v = 12;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 15;
    // v = 11;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 20;
    // v = 11;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // k = 30;
    // v = 11;
    // printf("insert(%zu)\n", k);
    // assert(!btree_insert(t, &k, &v));
    // dump_btree(t);

    // reset_btree(t);

    return 0;
}
