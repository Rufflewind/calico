#include <assert.h>
#include <stdlib.h>
#include <stdio.h> /* for printing error messages */
#include <string.h>
#include "malloca.h"

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

#include "binary_search.c"

static inline
int binary_sorted_search(const K *key,
                         const K *ptr,
                         size_t count,
                         size_t *pos_out)
{
    return binary_search(key,
                         ptr,
                         count,
                         sizeof(*ptr),
                         &generic_compare_K,
                         NULL,
                         pos_out);
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

/** A simple container used for readability purposes */
struct element_ref {
    K *key;
    V *value;
    leaf_node **child; /* right child */
};

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

/** It must actually be a branch, or this will cause UB. */
static inline
branch_node *unsafe_leaf_as_branch(leaf_node *m)
{
    return (branch_node *)m;
}

/** May return NULL if it's not a branch. */
static inline
branch_node *try_leaf_as_branch(int is_branch, leaf_node *m)
{
    if (!is_branch) {
        return NULL;
    }
    return unsafe_leaf_as_branch(m);
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

static inline
leaf_node *lookup_iter(size_t *i_out,
                       leaf_node *node,
                       size_t *h,
                       const K *key)
{
    int r = LOOKUP_METHOD(key, leaf_keys(node), *leaf_len(node), i_out);
    if (r || !--*h) {
        return NULL;
    }
    return branch_children(unsafe_leaf_as_branch(node))[*i_out];
}

/** Return the node and the position within that node. */
size_t raw_lookup_node(leaf_node **nodestack,
                       size_t *istack,
                       size_t height,
                       leaf_node *node,
                       const K *key)
{
    size_t h = height;
    assert(height);
    nodestack[0] = node;
    while ((node = lookup_iter(&istack[height - h],
                               nodestack[height - h],
                               &h,
                               key))) {
        nodestack[height - h] = node;
    }
    return height - h;
}

/** Return the node and the position within that node. */
leaf_node *find_node(size_t height,
                     leaf_node *node,
                     const K *key,
                     size_t *index)
{
    ++height;
    MALLOCA(size_t, istack, height);
    MALLOCA(leaf_node *, nodestack, height);
    size_t h = raw_lookup_node(nodestack, istack, height, node, key);
    leaf_node *ret = NULL;
    if (h != height) {
        ret = nodestack[h];
        *index = istack[h];
    }
    FREEA(nodestack);
    FREEA(istack);
    return ret;
}

/** Return the node and the position within that node. */
leaf_node *btree_get_node(btree *m, const K *key, size_t *index)
{
    if (!m->_root) {
        assert(m->_height == 0);
        assert(m->_len == 0);
        return NULL;
    }
    return find_node(m->_height - 1, m->_root, key, index);
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

static inline
void copy_elements(int is_branch,
                   leaf_node *restrict dstnode,
                   leaf_node *restrict srcnode,
                   size_t dstindex,
                   size_t srcindex,
                   size_t count)
{
    K *dks = leaf_keys(dstnode);
    K *sks = leaf_keys(srcnode);
    V *dvs = leaf_values(dstnode);
    V *svs = leaf_values(srcnode);
    memcpy(dks + dstindex, sks + srcindex, count * sizeof(*sks));
    memcpy(dvs + dstindex, svs + srcindex, count * sizeof(*svs));
    /* note that the we copy the RIGHT child, hence the "+ 1" */
    if (is_branch) {
        leaf_node **dcs = branch_children(unsafe_leaf_as_branch(dstnode)) + 1;
        leaf_node **scs = branch_children(unsafe_leaf_as_branch(srcnode)) + 1;
        memcpy(dcs + dstindex, scs + srcindex, count * sizeof(*scs));
    }
}

static inline
void move_elements(int is_branch,
                   leaf_node *node,
                   size_t dstindex,
                   size_t srcindex,
                   size_t count)
{
    K *ks = leaf_keys(node);
    V *vs = leaf_values(node);
    memmove(ks + dstindex, ks + srcindex, count * sizeof(*ks));
    memmove(vs + dstindex, vs + srcindex, count * sizeof(*vs));
    /* note that the we move the RIGHT child, hence the "+ 1" */
    if (is_branch) {
        leaf_node **cs = branch_children(unsafe_leaf_as_branch(node)) + 1;
        memmove(cs + dstindex, cs + srcindex, count * sizeof(*cs));
    }
}

static inline
void set_element(int is_branch,
                 leaf_node *node,
                 size_t index,
                 const struct element_ref *elem)
{
    leaf_keys(node)[index] = *elem->key;
    leaf_values(node)[index] = *elem->value;
    if (is_branch) {
        branch_children(unsafe_leaf_as_branch(node))[index + 1] = *elem->child;
    }
}

int insert_node_here(int is_branch,
                     leaf_node *node,
                     size_t i,
                     /* the `const` here is just placebo, but still: don't try
                        to modify the values that its members point to*/
                     const struct element_ref *elem,
                     struct element_ref *elem_out)
{
    size_t len = *leaf_len(node);

    /* case A: enough room to do a simple insert */
    if (len < 2 * B - 1) {
        move_elements(is_branch, node, i + 1, i, len - i);
        set_element(is_branch, node, i, elem);
        ++*leaf_len(node);
        return 0;
    }

    /* case B: not enough room; need to split node */
    assert(len == 2 * B - 1);
    leaf_node *newnode = (leaf_node *)malloc(
        is_branch ?
        sizeof(branch_node) :
        sizeof(leaf_node));
    if (!newnode) {
        /* FIXME: we should return 1 instead of failing, but we'd need to
           rollback the incomplete insert, which is tricky :c */
        fprintf(stderr, "%s:%lu: Out of memory\n",
                __FILE__, (unsigned long)__LINE__);
        fflush(stderr);
        abort();
    }
    size_t s = i > B ? i : B;
    copy_elements(is_branch, newnode, node, s - B, s, B * 2 - 1 - s);
    if (i == B) {
        if (is_branch) {
            branch_children((branch_node *)newnode)[0] = *elem->child;
        }
        *elem_out->key = *elem->key;
        *elem_out->value = *elem->value;
    } else {
        size_t mid = i < B ? B - 1 : B;
        K midkey = leaf_keys(node)[mid];;
        V midvalue = leaf_values(node)[mid];
        if (is_branch) {
            branch_children((branch_node *)newnode)[0] =
                branch_children((branch_node *)node)[mid + 1];
        }
        if (i < B) {
            move_elements(is_branch, node, i + 1, i, B - 1 - i);
            set_element(is_branch, node, i, elem);
        } else {
            copy_elements(is_branch, newnode, node, 0, B + 1, i - B - 1);
            set_element(is_branch, newnode, i - B - 1, elem);
        }
        *elem_out->key = midkey;
        *elem_out->value = midvalue;
    }
    *leaf_len(node) = B;
    *leaf_len(newnode) = B - 1;
    *elem_out->child = newnode;
    return -2;
}

int insert_node(size_t height,
                leaf_node *node,
                const K *key,
                const V *value,
                K *key_inout,
                V *value_inout,
                leaf_node **child_inout)
{
    int r;
    size_t h = 0;
    ++height;
    MALLOCA(size_t, istack, height);
    MALLOCA(leaf_node *, nodestack, height);
    h = raw_lookup_node(nodestack, istack, height, node, key);
    if (h != height) {
        leaf_values(nodestack[h])[istack[h]] = *value;
        r = -1;
    } else {
        /* the rest of it does not depend on the comparison operation, only on
           the layout of the structure */
        h = height - 1;
        struct element_ref elem = {(K *)key, (V *)value, child_inout};
        struct element_ref elem_out = {key_inout, value_inout, child_inout};
        r = insert_node_here(0, nodestack[h], istack[h],
                             &elem, &elem_out);
        while (h-- && r < -1) {
            r = insert_node_here(1, nodestack[h], istack[h],
                                 &elem_out, &elem_out);
        }
    }
    FREEA(nodestack);
    FREEA(istack);
    return r;
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
