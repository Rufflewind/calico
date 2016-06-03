/** @file

Associative arrays implemented using B-trees.

This is a template header.  All identifiers in this are prefixed with an
arbitrary prefix provided by the user.  Before including this header,
`<calico/btree_head.h>` must be included as well.

The main parameters are:

  - `K`: key type
  - `V`: value type
  - `CompareFunction`: function used to compare keys
    (default: `cal_pcmp`, type: `int (*)(const K *, const K *))`)

Some other parameters useful for tuning performance are:

  - `B`: number of children per node (default: 8, minimum: 2)
  - `SearchFunction`: function used search for a key in a node
     (default: `linear_sorted_search`)
  - `ChildIndexType`: type used to store indices of child nodes
    (default: `unsigned short`)
  - `HeightType`: type used to store the height of the tree
    (default: `unsigned char`)

Here's an example:

~~~c
#include <string.h>
#include <calico/btree_head.h>

#define Prefix id
#define K int
#define V double
#include <calico/btree_template.h>

#define Prefix si
#define K const char *
#define V int
#define CompareFunction strcmp
#include <calico/btree_template.h>

int main(void)
{
    int i = 42;
    double d = 1.5;
    id_btree t1;
    si_btree t2;

    id_btree_init(&t1);
    id_btree_insert(&t2, &i, &d);
    id_btree_reset(&t1);

    si_btree_init(&t2);
    si_btree_insert(&t2, &"hello", &i);
    si_btree_reset(&t2);

    return 0;
}
~~~

*/

typedef struct {
cal_cond_ndebug(,
    /* when assertions are enabled, track whether it's a branch or leaf to
       catch bugs more easily */
    int _is_branch;
)
    /* the number of valid keys (or number of valid values),
       ranging from 0 to 2 * B - 1 */
    ChildIndexType _len;
    /* an array of keys, with [0, _len) valid */
    K _keys[2 * B - 1];
cal_cond(HasValue)(
    /* an array of values, with [0, _len) valid */
    V _values[2 * B - 1];
,)
} leaf_node;

typedef struct {
    /* we use `leaf_node` as a "base type" */
    leaf_node _data;
    /* child nodes, with [0, _data._len] valid */
    leaf_node *_children[2 * B];
} branch_node;

/** An associative array data structure backed by a B-tree. */
typedef struct {
    leaf_node *_root;
    ChildIndexType _len;
    HeightType _height;
} btree;

/* log2(UINTPTR_MAX / sizeof(leaf_node)) / log2(B) + 1 */
enum {
    MAX_HEIGHT =
        (CHAR_BIT * sizeof(void *) - cal_minlog2(sizeof(leaf_node)))
        / cal_minlog2(B) + 1
};

static_assert((HeightType)MAX_HEIGHT == MAX_HEIGHT, "height is too big");
static_assert((ChildIndexType)B == B, "B is too big");
static_assert(B >= 2, "B must be at least 2");

/** Refers to a specific location in a B-tree.  If the entry is vacant, then
    an element may be inserted at the location.  If the entry is occupied,
    then its value may be read or modified, or the element may be removed
    entirely. */
typedef struct {
    leaf_node *_nodestack[MAX_HEIGHT];
    ChildIndexType _istack[MAX_HEIGHT];
    HeightType _depth;
} btree_entry;

/* A simple container used for readability purposes */
struct elem_ref {
    K *key;
cal_cond(HasValue)(
    V *value;
,)
    leaf_node **child; /* right child */
};

static inline
leaf_node *alloc_leaf(void)
{
    leaf_node *m = (leaf_node *)malloc(sizeof(*m));
    if (m) {
        assert(!(m->_is_branch = 0));
        m->_len = 0;
    }
    return m;
}

static inline
ChildIndexType *leaf_len(leaf_node *m)
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
    return cal_cond(HasValue)(m->_values, leaf_keys(m));
}

static inline
branch_node *alloc_branch(void)
{
    branch_node *m = (branch_node *)malloc(sizeof(*m));
    if (m) {
        assert(m->_data._is_branch = 1);
        m->_data._len = 0;
    }
    return m;
}

static inline
leaf_node *branch_as_leaf(branch_node *m)
{
    return &m->_data;
}

static inline
leaf_node **branch_children(branch_node *m)
{
    return m->_children;
}

static inline
ChildIndexType *branch_len(branch_node *m)
{
    return leaf_len(branch_as_leaf(m));
}

static inline
K *branch_keys(branch_node *m)
{
    return leaf_keys(branch_as_leaf(m));
}

static inline
V *branch_values(branch_node *m)
{
    leaf_node *ml = branch_as_leaf(m);
    return cal_cond(HasValue)(leaf_values, leaf_keys)(ml);
}

/* It must actually be a branch, or this will cause UB. */
static inline
branch_node *unsafe_leaf_as_branch(leaf_node *m)
{
    assert(m->_is_branch);
    return (branch_node *)m;
}

/* It must actually be a branch, or this will cause UB. */
static inline
leaf_node **unsafe_leaf_children(leaf_node *m)
{
    return branch_children(unsafe_leaf_as_branch(m));
}

/* May return NULL if it's not a branch. */
static inline
branch_node *try_leaf_as_branch(int is_branch, leaf_node *m)
{
    assert(m);
    assert(!!is_branch == m->_is_branch);
    if (!is_branch) {
        return NULL;
    }
    return unsafe_leaf_as_branch(m);
}

static inline
struct elem_ref node_elems(int is_branch, leaf_node *m)
{
    struct elem_ref r = {
        leaf_keys(m),
        leaf_values(m),
        /* we always manipulate the key+value with its RIGHT child */
        is_branch ? unsafe_leaf_children(m) + 1 : NULL
    };
    return r;
}

/** Initializes an empty `btree`. */
static inline
void btree_init(btree *m)
{
    m->_len = 0;
    m->_height = 0;
    m->_root = NULL;
}

static inline
void free_node(HeightType height, leaf_node *m)
{
    branch_node *mb = try_leaf_as_branch(height, m);
    if (mb) {
        ChildIndexType i;
        for (i = 0; i < *leaf_len(m) + 1; ++i) {
            free_node((HeightType)(height - 1), branch_children(mb)[i]);
        }
    }
    free(m);
}

/** Removes all elements from a `btree` and frees all memory consumed by the
    B-tree nodes.  This operation does not read any of the keys.

    @note If the keys or values are associated with resources that need to be
    released, `btree_reset` does *not* release them!  In this situation, one
    must manually iterate over each element and release them:
    ~~~c
    for (btree_find_first(m, &entry);
         btree_entry_occupied(m, &entry);
         btree_entry_next(m, &entry)) {
        free(btree_entry_key(&entry));
        free(btree_entry_get(&entry));
    }
    btree_reset(m);
    ~~~

*/
static inline
void btree_reset(btree *m)
{
    if (m->_root) {
        free_node((HeightType)(m->_height - 1), m->_root);
    }
    btree_init(m);
}

/** Gets the number of elements.  This is an O(1) operation. */
static inline
size_t btree_len(const btree *m)
{
    return m->_len;
}

static inline
int generic_compare(void *ctx, const void *x, const void *y)
{
    (void)ctx;
    return CompareFunction((const K *)x, (const K *)y);
}

static inline
leaf_node *lookup_iter(ChildIndexType *i_out,
                       leaf_node *node,
                       HeightType *h,
                       HeightType height,
                       const K *key)
{
    size_t i;
    int r;
    assert(height);
    r = SearchFunction(key,
                       leaf_keys(node),
                       *leaf_len(node),
                       sizeof(*key),
                       &generic_compare,
                       NULL,
                       &i);
    *i_out = (ChildIndexType)i;
    if (r || ++*h >= height) {
        return NULL;
    }
    return branch_children(unsafe_leaf_as_branch(node))[*i_out];
}

/* Return the node and the position within that node. */
static inline
HeightType raw_lookup_node(leaf_node **nodestack,
                           ChildIndexType *istack,
                           HeightType height,
                           leaf_node *node,
                           const K *key)
{
    HeightType h = 0;
    assert(key);
    assert(height);
    nodestack[0] = node;
    while ((node = lookup_iter(&istack[h], nodestack[h], &h, height, key))) {
        nodestack[h] = node;
    }
    return h;
}

/** Searches for the given `key`.  The entry returned can be either occupied
    or vacant, which indicates whether the `key` was found. */
static inline
void btree_find(btree *m, const K *key, btree_entry *entry_out)
{
    assert(entry_out);
    assert(m->_height <= MAX_HEIGHT);
    entry_out->_depth = 0;
    if (m->_height) {
        entry_out->_depth = raw_lookup_node(
            entry_out->_nodestack,
            entry_out->_istack,
            m->_height,
            m->_root,
            key
        );
    }
}

static inline
void move_to_extremum(HeightType height,
                      leaf_node *node,
                      ChildIndexType which,
                      HeightType *depth,
                      ChildIndexType *istack,
                      leaf_node **nodestack)
{
    HeightType h;
    for (h = 0; ; ++h) {
        ChildIndexType i = (ChildIndexType)(*leaf_len(node) * !!which);
        if (which && h >= height - 1) {
            --i;
        }
        istack[h] = i;
        nodestack[h] = node;
        if (h >= height - 1) {
            break;
        }
        node = unsafe_leaf_children(node)[i];
    }
    *depth = (HeightType)(*depth + height - 1);
}

static inline
void find_extremum(btree *m, ChildIndexType which, btree_entry *entry_out)
{
    HeightType height = m->_height;
    assert(height <= MAX_HEIGHT);
    entry_out->_depth = 0;
    if (height) {
        move_to_extremum(height, m->_root, which, &entry_out->_depth,
                         entry_out->_istack, entry_out->_nodestack);
    }
}

/** Locates the first (least) entry.  If the B-tree is empty, a vacant entry
    is returned. */
static inline
void btree_find_first(btree *m, btree_entry *entry_out)
{
    find_extremum(m, 0, entry_out);
}

/** Locates the last (greatest) entry.  If the B-tree is empty, a vacant entry
    is returned. */
static inline
void btree_find_last(btree *m, btree_entry *entry_out)
{
    find_extremum(m, 1, entry_out);
}

/** Returns whether a entry refers to an existing element. */
static inline
int btree_entry_occupied(const btree *m, const btree_entry *ent)
{
    assert(ent->_depth <= m->_height); /* make sure it's valid */
    return ent->_depth != m->_height;
}

/** Gets a pointer to the key at an occupied entry.  If the entry is not
    occupied, the behavior is undefined.

    @note The key can be modified as long as it remains semantically equal to
    its original value.  Otherwise, if the key is modified to a different
    value, or otherwise made impossible to compare (e.g. by deallocating
    string keys), then any operation that involves key comparison will cause
    undefined behavior.  These operations include `btree_get`,
    `btree_get_const`, `btree_insert`, and `btree_remove`.  It remains safe to
    call `btree_len`, `btree_reset`, or the `btree_entry`-related
    functions.  */
static inline
K *btree_entry_key(const btree_entry *ent)
{
    return
        leaf_keys(ent->_nodestack[ent->_depth]) +
        ent->_istack[ent->_depth];
}

/** Gets a pointer to the value at an occupied entry.  If the entry is not
    occupied, the behavior is undefined. */
static inline
V *btree_entry_get(const btree_entry *ent)
{
    return
        leaf_values(ent->_nodestack[ent->_depth]) +
        ent->_istack[ent->_depth];
}

/** Modifies an occupied entry to refer to its next element.  If there are no
    more elements, the next entry is vacant.  Otherwise, it is occupied.  The
    argument `ent` must point to an occupied entry, or the behavior is
    undefined.  Returns `1` if the new entry is occupied, `0` if the new entry
    is vacant. */
static inline
int btree_entry_next(const btree *m, btree_entry *ent)
{
    HeightType height = m->_height;
    assert(btree_entry_occupied(m, ent));
    ++ent->_istack[ent->_depth];
    if (ent->_depth < height - 1) {
        ++ent->_depth;
        move_to_extremum(
            (HeightType)(height - ent->_depth),
            unsafe_leaf_children(ent->_nodestack[ent->_depth - 1])
                [ent->_istack[ent->_depth - 1]],
            0,
            &ent->_depth,
            ent->_istack + ent->_depth,
            ent->_nodestack + ent->_depth
        );
    } else {
        assert(ent->_istack[ent->_depth] <=
               *leaf_len(ent->_nodestack[ent->_depth]));
        while (ent->_istack[ent->_depth] ==
               *leaf_len(ent->_nodestack[ent->_depth])) {
            if (!ent->_depth) {
                ent->_depth = height;
                return 0;
            }
            --ent->_depth;
        }
    }
    return 1;
}

/** Modifies an occupied entry to refer to its previous element.  If there are
    no more elements, the previous entry is vacant.  Otherwise, it is
    occupied.  The argument `ent` must point to an occupied entry, or the
    behavior is undefined. */
static inline
int btree_entry_prev(const btree *m, btree_entry *ent)
{
    HeightType height = m->_height;
    assert(btree_entry_occupied(m, ent));
    if (ent->_depth < height - 1) {
        ++ent->_depth;
        move_to_extremum(
            (HeightType)(height - ent->_depth),
            unsafe_leaf_children(ent->_nodestack[ent->_depth - 1])
                [ent->_istack[ent->_depth - 1]],
            1,
            &ent->_depth,
            ent->_istack + ent->_depth,
            ent->_nodestack + ent->_depth
        );
    } else {
        while (!ent->_istack[ent->_depth]) {
            if (!ent->_depth) {
                ent->_depth = height;
                return 0;
            }
            --ent->_depth;
        }
        --ent->_istack[ent->_depth];
    }
    return 1;
}

static inline
leaf_node *find_node(HeightType height,
                     leaf_node *node,
                     const K *key,
                     ChildIndexType *index)
{
    ChildIndexType i;
    HeightType h = 0;
    leaf_node *newnode;
    while ((newnode = lookup_iter(&i, node, &h, height, key))) {
        node = newnode;
    }
    if (h == height) {
        return NULL;
    }
    *index = i;
    return node;
}

/* Return the node and the position within that node. */
static inline
leaf_node *get_node(btree *m, const K *key, ChildIndexType *index)
{
    if (!m->_root) {
        assert(m->_height == 0);
        assert(m->_len == 0);
        return NULL;
    }
    return find_node(m->_height, m->_root, key, index);
}

/** Retrieves the value with the given key.  Returns `NULL` if it is not
    found. */
static inline
V *btree_get(btree *m, const K *k)
{
    ChildIndexType i;
    leaf_node *n;
    n = get_node(m, k, &i);
    if (!n) {
        return NULL;
    }
    return leaf_values(n) + i;
}

/** Retrieves the value with the given key.  Returns `NULL` if it is not
    found. */
static inline
const V *btree_get_const(const btree *m, const K *k)
{
    return btree_get((btree *)m, k);
}

/** Returns whether the the given key exists in the `btree`. */
static inline
int btree_contains_key(const btree *m, const K *k)
{
    ChildIndexType i;
    return !!get_node((btree *)m, k, &i);
}

static inline
void copy_elems(struct elem_ref dst, struct elem_ref src, size_t count)
{
    /* use memmove due to potential for overlap; it's not always needed, but
       better safe than sorry and the performance impact here is minimal */
    memmove(dst.key, src.key, count * sizeof(*src.key));
    memmove(dst.value, src.value, count * sizeof(*src.value));
    assert(!!dst.child == !!src.child);
    if (src.child) {
        memmove(dst.child, src.child, count * sizeof(*src.child));
    }
}

static inline
struct elem_ref offset_elem(struct elem_ref dst, ptrdiff_t count)
{
    struct elem_ref r = {
        dst.key + count,
        dst.value + count,
        dst.child ? dst.child + count : NULL
    };
    return r;
}

static inline
int insert_node_here(int is_branch,
                     leaf_node *node,
                     ChildIndexType i,
                     struct elem_ref elem,
                     struct elem_ref elem_out)
{
    ChildIndexType len = *leaf_len(node);

    /* case A: enough room to do a simple insert */
    struct elem_ref elems = node_elems(is_branch, node);
    if (len < 2 * B - 1) {
        copy_elems(offset_elem(elems, i + 1), offset_elem(elems, i),
                   (size_t)(len - i));
        copy_elems(offset_elem(elems, i), elem, 1);
        ++*leaf_len(node);
        return 0;
    }

    /* case B: not enough room; need to split node */
    assert(len == 2 * B - 1);
    leaf_node *newnode =
        is_branch ?
        branch_as_leaf(alloc_branch()) :
        alloc_leaf();
    if (!newnode) {
        /* FIXME: we should return 1 instead of failing, but we'd need to
           rollback the incomplete insert, which is tricky :c */
        fprintf(stderr, "%s:%lu: Out of memory\n",
                __FILE__, (unsigned long)__LINE__);
        fflush(stderr);
        abort();
    }
    struct elem_ref newelems = node_elems(is_branch, newnode);
    size_t s = i > B ? i : B;
    copy_elems(offset_elem(newelems, (ptrdiff_t)(s - B)),
               offset_elem(elems, (ptrdiff_t)s),
               B * 2 - 1 - s);
    if (i == B) {
        if (is_branch) {
            unsafe_leaf_children(newnode)[0] = *elem.child;
        }
        *elem_out.key = *elem.key;
        *elem_out.value = *elem.value;
    } else {
        ChildIndexType mid = i < B ? B - 1 : B;
        K midkey = leaf_keys(node)[mid];
        V midvalue = leaf_values(node)[mid];
        if (is_branch) {
            unsafe_leaf_children(newnode)[0] =
                unsafe_leaf_children(node)[mid + 1];
        }
        if (i < B) {
            copy_elems(offset_elem(elems, i + 1), offset_elem(elems, i),
                       (size_t)B - 1 - i);
            copy_elems(offset_elem(elems, i), elem, 1);
        } else {
            copy_elems(newelems, offset_elem(elems, B + 1),
                       i - (size_t)B - 1);
            copy_elems(offset_elem(newelems, i - B - 1), elem, 1);
        }
        *elem_out.key = midkey;
        *elem_out.value = midvalue;
    }
    *leaf_len(node) = B;
    *leaf_len(newnode) = B - 1;
    *elem_out.child = newnode;
    return -2;
}

/** Inserts an element into the `btree` at a vacant entry.  If the entry is
    not vacant, the behavior is undefined.

    @note The `key` must match the original key used to obtain the vacant
    entry, or the behavior is undefined. */
static inline
int btree_entry_insert(btree *m, btree_entry *entry,
                       const K *key, const V *value)
{
    K newkey;
    V newvalue;
    leaf_node *newchild;
    struct elem_ref newelem = {&newkey, &newvalue, &newchild};
    struct elem_ref elem = {(K *)key, (V *)value, NULL};
    HeightType h, height = m->_height;
    int r;
    if (!height) {
        assert(!m->_len);
        assert(!m->_root);
        m->_root = alloc_leaf();
        if (!m->_root) {
            return 1;
        }
        ++m->_len;
        m->_height = 1;
        *leaf_len(m->_root) = 1;
        leaf_keys(m->_root)[0] = *key;
        leaf_values(m->_root)[0] = *value;
        return 0;
    }
    h = (HeightType)(height - 1);
    r = insert_node_here(0, entry->_nodestack[h], entry->_istack[h],
                         elem, newelem);
    while (h-- && r < -1) {
        r = insert_node_here(1, entry->_nodestack[h], entry->_istack[h],
                             newelem, newelem);
    }
    if (r > 0) {
        return r;
    }
    if (r < 0) {
        branch_node *newroot = alloc_branch();
        if (!newroot) {
            /* FIXME: we should return 1 instead of failing, but we'd need
               to rollback the incomplete insert, which is tricky :c */
            fprintf(stderr, "%s:%lu: Out of memory\n",
                    __FILE__, (unsigned long)__LINE__);
            fflush(stderr);
            abort();
        }
        ++m->_height;
        *branch_len(newroot) = 1;
        branch_keys(newroot)[0] = newkey;
        branch_values(newroot)[0] = newvalue;
        branch_children(newroot)[0] = m->_root;
        branch_children(newroot)[1] = newchild;
        m->_root = branch_as_leaf(newroot);
        r = 0;
    }
    ++m->_len;
    return 0;
}

/** Inserts an element into the `btree`.  If an element with the same `key`
    already exists, its value is replaced (the key remains unchanged,
    however).

    @note If the existing value that is replaced is associated with some
    resources, they will not be released.

    @todo Add an argument to retrieve the old value (maybe call it
    `insert_swap`?).
 */
static inline
int btree_insert(btree *m, const K *key, const V *value)
{
    btree_entry ent;
    btree_find(m, key, &ent);
    if (btree_entry_occupied(m, &ent)) {
        *btree_entry_get(&ent) = *value;
        return 0;
    }
    return btree_entry_insert(m, &ent, key, value);
}

/* to get the left parent, use index = i - 1;
   to get the right parent, use index = i */
static inline
struct elem_ref parent_elem(int is_branch,
                            branch_node *parentnode,
                            ChildIndexType index)
{
    assert(index < *branch_len(parentnode) + 1);
    leaf_node *right_child = branch_children(parentnode)[index + 1];
    struct elem_ref r = {
        branch_keys(parentnode) + index,
        branch_values(parentnode) + index,
        is_branch ? unsafe_leaf_children(right_child) : NULL
    };
    return r;
}

static inline
int steal_left(int is_branch,
               leaf_node *node,
               branch_node *parentnode,
               ChildIndexType i,
               ChildIndexType previ)
{
    ChildIndexType len, leftlen, lensum, dleftlen, newleftlen, newlen;
    struct elem_ref elems, leftelems, parentelem;
    leaf_node *leftnode;

    /* make sure previ is a valid index */
    assert(previ <= *branch_len(parentnode));

    if (!previ) {
        /* there is no left neighbor */
        return 0;
    }

    leftnode = branch_children(parentnode)[previ - 1];
    leftlen = *leaf_len(leftnode);

    if (leftlen < B) {
        /* left neighbor doesn't have enough elements to steal */
        return 0;
    }

    elems = node_elems(is_branch, node);
    leftelems = node_elems(is_branch, leftnode);
    parentelem = parent_elem(is_branch, parentnode,
                             (ChildIndexType)(previ - 1));
    len = *leaf_len(node);
    lensum = (ChildIndexType)(leftlen + len - 1);
    newlen = lensum / 2;
    newleftlen = (ChildIndexType)(lensum - newlen);
    dleftlen = (ChildIndexType)(leftlen - newleftlen);

    copy_elems(offset_elem(elems, dleftlen + i),
               offset_elem(elems, i + 1),
               (ChildIndexType)(len - i - 1));
    copy_elems(offset_elem(elems, dleftlen),
               elems,
               i);
    copy_elems(offset_elem(elems, dleftlen - 1),
               parentelem,
               1);
    copy_elems(elems,
               offset_elem(leftelems, newleftlen + 1),
               (ChildIndexType)(dleftlen - 1));
    copy_elems(parentelem,
               offset_elem(leftelems, newleftlen),
               1);

    *leaf_len(leftnode) = newleftlen;
    *leaf_len(node) = newlen;

    return 1;
}

static inline
int steal_right(int is_branch,
                leaf_node *node,
                branch_node *parentnode,
                ChildIndexType i,
                ChildIndexType previ)
{
    ChildIndexType len, rightlen, lensum, drightlen, newrightlen, newlen;
    struct elem_ref elems, rightelems, parentelem;
    leaf_node *rightnode;

    /* make sure previ is a valid index */
    assert(previ <= *branch_len(parentnode));

    /* this condition can happen if we are at the rightmost child but the left
       neighbor does not contain enough elements to steal from */
    if (previ == *branch_len(parentnode)) {
        /* there is no right neighbor */
        return 0;
    }

    rightnode = branch_children(parentnode)[previ + 1];
    rightlen = *leaf_len(rightnode);

    if (rightlen < B) {
        /* right neighbor doesn't have enough elements to steal */
        return 0;
    }

    elems = node_elems(is_branch, node);
    rightelems = node_elems(is_branch, rightnode);
    parentelem = parent_elem(is_branch, parentnode, previ);
    len = *leaf_len(node);
    lensum = (ChildIndexType)(rightlen + *leaf_len(node) - 1);
    newlen = lensum / 2;
    newrightlen = (ChildIndexType)(lensum - newlen);
    drightlen = (ChildIndexType)(rightlen - newrightlen);

    copy_elems(offset_elem(elems, i),
               offset_elem(elems, i + 1),
               (ChildIndexType)(len - 1 - i));
    copy_elems(offset_elem(elems, len - 1),
               parentelem,
               1);
    copy_elems(offset_elem(elems, len),
               rightelems,
               (ChildIndexType)(drightlen - 1));
    copy_elems(parentelem,
               offset_elem(rightelems, drightlen - 1),
               1);
    copy_elems(rightelems,
               offset_elem(rightelems, drightlen),
               newrightlen);

    *leaf_len(rightnode) = newrightlen;
    *leaf_len(node) = newlen;
    return 1;
}

static inline
int merge_left(int is_branch,
               leaf_node *node,
               branch_node *parentnode,
               ChildIndexType i,
               ChildIndexType previ,
               ChildIndexType *oldindex)
{
    ChildIndexType leftlen, len;
    struct elem_ref elems, leftelems, parentelem;
    leaf_node *leftnode;

    /* make sure previ is a valid index */
    assert(previ <= *branch_len(parentnode));

    if (!previ) {
        /* there is no left neighbor */
        return 0;
    }

    leftnode = branch_children(parentnode)[previ - 1];
    leftlen = *leaf_len(leftnode);

    /* left neighbor must not have too many elements */
    assert(leftlen == B - 1);

    len = *leaf_len(node);
    elems = node_elems(is_branch, node);
    leftelems = node_elems(is_branch, leftnode);
    parentelem = parent_elem(is_branch, parentnode,
                             (ChildIndexType)(previ - 1));

    copy_elems(offset_elem(leftelems, leftlen),
               parentelem,
               1);
    copy_elems(offset_elem(leftelems, leftlen + 1),
               elems,
               i);
    copy_elems(offset_elem(leftelems, leftlen + 1 + i),
               offset_elem(elems, i + 1),
               (size_t)(len - i - 1));

    free(node);
    *leaf_len(leftnode) = (ChildIndexType)(leftlen + len);
    *oldindex = (ChildIndexType)(previ - 1);
    return 1;
}

static inline
int merge_right(int is_branch,
                leaf_node *node,
                branch_node *parentnode,
                ChildIndexType i,
                ChildIndexType previ,
                ChildIndexType *oldindex)
{
    ChildIndexType rightlen, len;
    struct elem_ref elems, rightelems, parentelem;
    leaf_node *rightnode;

    /* make sure previ is a valid index */
    assert(previ <= *branch_len(parentnode));

    /* make sure there is a right neighbor */
    assert(previ != *branch_len(parentnode));

    rightnode = branch_children(parentnode)[previ + 1];
    rightlen = *leaf_len(rightnode);

    /* right neighbor must not have too many elements */
    assert(rightlen == B - 1);

    len = *leaf_len(node);
    elems = node_elems(is_branch, node);
    rightelems = node_elems(is_branch, rightnode);
    parentelem = parent_elem(is_branch, parentnode, previ);

    copy_elems(offset_elem(elems, i),
               offset_elem(elems, i + 1),
               (size_t)(len - i - 1));
    copy_elems(offset_elem(elems, len - 1),
               parentelem,
               1);
    copy_elems(offset_elem(elems, len),
               rightelems,
               rightlen);

    free(rightnode);
    *leaf_len(node) = (ChildIndexType)(len + rightlen);
    *oldindex = previ;
    return 1;
}

/** Removes the element at an occupied entry.  The entry is destroyed in the
    process.  If the entry is not occupied, the behavior is undefined. */
static inline
void btree_entry_remove(btree *m, btree_entry *ent)
{
    HeightType depth = ent->_depth;
    HeightType height = m->_height;

    /* a valid entry cannot belong to a tree of zero height (no elements) */
    assert(height);

    /* iterator must point to an exact match (as opposite to an "in-between"
       match, which is useful only for inserting elements) */
    assert(btree_entry_occupied(m, ent));

    --m->_len;

    /* if node is not leaf, swap with the nearest leaf to the left and fill
       the entry stacks completely throughout [0, height) */
    if (depth < height - 1) {
        HeightType h = depth;
        leaf_node *uppernode = ent->_nodestack[depth];
        ChildIndexType upperi = ent->_istack[depth];
        K *key = &leaf_keys(uppernode)[upperi];
        leaf_node *node = uppernode;
        ChildIndexType i = upperi;
        ++h;
        do {
            node = branch_children(unsafe_leaf_as_branch(node))[i];
            ent->_nodestack[h] = node;
            i = *leaf_len(node);
            ent->_istack[h] = i;
            ++h;
        } while (h < height);
        --ent->_istack[height - 1];
        leaf_node *lowernode = ent->_nodestack[height - 1];
        ChildIndexType loweri = ent->_istack[height - 1];
        *key = leaf_keys(lowernode)[loweri];
        leaf_values(uppernode)[upperi] = leaf_values(lowernode)[loweri];
        depth = (HeightType)(height - 1);
    }

    ChildIndexType i = ent->_istack[depth];
    leaf_node *node = ent->_nodestack[depth];
    while (1) {
        ChildIndexType len = *leaf_len(node);
        int is_branch = depth != height - 1;

        /* easy case: no underflow (or is root node) */
        if (len >= B || !depth) {
            struct elem_ref elem = node_elems(is_branch, node);
            copy_elems(offset_elem(elem, i),
                       offset_elem(elem, i + 1),
                       (size_t)(len - i - 1));
            *leaf_len(node) = (ChildIndexType)(len - 1);
            if (!depth && len == 1 && is_branch) {
                /* root node is a branch and is about to become empty */
                m->_root = unsafe_leaf_children(node)[0];
                free(node);
                --m->_height;
            }
            break;
        }

        branch_node *parentnode =
            unsafe_leaf_as_branch(ent->_nodestack[depth - 1]);
        ChildIndexType previ = ent->_istack[depth - 1];

        if (steal_left(is_branch, node, parentnode, i, previ) ||
            steal_right(is_branch, node, parentnode, i, previ)) {
            break;
        }

        (void)(merge_left(is_branch, node, parentnode, i, previ, &i) ||
               merge_right(is_branch, node, parentnode, i, previ, &i));

        --depth;
        node = branch_as_leaf(parentnode);
    }
}

/** Removes the element at the given key and stores the results in `*key_out`
    and `*value_out` respectively, returning `1`.  If the element does not
    exist, does nothing and returns `0`.

    If the key and/or value are associated with resources, it may be necessary
    to release them if `btree_remove` returns `1`, like this:

    ~~~c
    if (btree_remove(m, key, &old_key, &old_value)) {
        free(old_key);
        free(old_value);
    }
    ~~~

    `key` and `key_out` are permitted to alias.

*/
static inline
int btree_remove(btree *m, const K *key, K *key_out, V *value_out)
{
    btree_entry ent;
    btree_find(m, key, &ent);
    if (!btree_entry_occupied(m, &ent)) {
        return 0;
    }
    if (key_out) {
        *key_out = *btree_entry_key(&ent);
    }
    if (value_out) {
        *value_out = *btree_entry_get(&ent);
    }
    btree_entry_remove(m, &ent);
    return 1;
}
