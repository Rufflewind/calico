static inline
int generic_compare(void *ctx, const void *x, const void *y)
{
    (void)ctx;
    return CompareFunction((const K *)x, (const K *)y);
}

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

/** A simple container used for readability purposes */
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
    return HasValue ? m->_values : NULL;
}

typedef struct {
    /* we use `leaf_node` as a "base type" */
    leaf_node _data;
    /* child nodes, with [0, _data._len] valid */
    leaf_node *_children[2 * B];
} branch_node;

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
    return HasValue ? leaf_values(branch_as_leaf(m)) : NULL;
}

/** It must actually be a branch, or this will cause UB. */
static inline
branch_node *unsafe_leaf_as_branch(leaf_node *m)
{
    assert(m->_is_branch);
    return (branch_node *)m;
}

/** It must actually be a branch, or this will cause UB. */
static inline
leaf_node **unsafe_leaf_children(leaf_node *m)
{
    return branch_children(unsafe_leaf_as_branch(m));
}

/** May return NULL if it's not a branch. */
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

typedef struct {
    leaf_node *_nodestack[MAX_HEIGHT];
    ChildIndexType _istack[MAX_HEIGHT];
    HeightType _depth;
} btree_cursor;

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
        for (ChildIndexType i = 0; i < *leaf_len(m) + 1; ++i) {
            free_node(height - 1, branch_children(mb)[i]);
        }
    }
    free(m);
}

static inline
void btree_reset(btree *m)
{
    if (m->_root) {
        free_node(m->_height - 1, m->_root);
    }
    btree_init(m);
}

static inline
size_t btree_len(const btree *m)
{
    return m->_len;
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

/** Return the node and the position within that node. */
static inline
HeightType raw_lookup_node(leaf_node **nodestack,
                            ChildIndexType *istack,
                            HeightType height,
                            leaf_node *node,
                            const K *key)
{
    HeightType h = 0;
    assert(height);
    nodestack[0] = node;
    while ((node = lookup_iter(&istack[h], nodestack[h], &h, height, key))) {
        nodestack[h] = node;
    }
    return h;
}

static inline
int btree_lookup(btree_cursor *cur, btree *m, const K *key)
{
    assert(m->_height <= MAX_HEIGHT);
    if (!m->_height) {
        cur->_depth = 0;
        return 0;
    }
    cur->_depth = raw_lookup_node(cur->_nodestack,
                                  cur->_istack,
                                  m->_height,
                                  m->_root,
                                  key);
    return cur->_depth != m->_height;
}

/** Return the node and the position within that node. */
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

/** Return the node and the position within that node. */
static inline
leaf_node *btree_get_node(btree *m, const K *key, ChildIndexType *index)
{
    if (!m->_root) {
        assert(m->_height == 0);
        assert(m->_len == 0);
        return NULL;
    }
    return find_node(m->_height, m->_root, key, index);
}

static inline
V *btree_get(btree *m, const K *k)
{
    ChildIndexType i;
    leaf_node *n;
    if (!HasValue) {
        return NULL;
    }
    n = btree_get_node(m, k, &i);
    if (!n) {
        return NULL;
    }
    return leaf_values(n) + i;
}

static inline
const V *btree_get_const(const btree *m, const K *k)
{
    return btree_get((btree *)m, k);
}

static inline
int btree_in(const btree *m, const K *k)
{
    ChildIndexType i;
    return !!btree_get_node((btree *)m, k, &i);
}

static inline
void copy_elems(struct elem_ref dst,
                struct elem_ref src,
                size_t count)
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
struct elem_ref offset_elem(struct elem_ref dst, size_t count)
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
        copy_elems(offset_elem(elems, i + 1), offset_elem(elems, i), len - i);
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
    copy_elems(offset_elem(newelems, s - B), offset_elem(elems, s),
               B * 2 - 1 - s);
    if (i == B) {
        if (is_branch) {
            unsafe_leaf_children(newnode)[0] = *elem.child;
        }
        *elem_out.key = *elem.key;
        *elem_out.value = *elem.value;
    } else {
        ChildIndexType mid = i < B ? B - 1 : B;
        K midkey = leaf_keys(node)[mid];;
        V midvalue = leaf_values(node)[mid];
        if (is_branch) {
            unsafe_leaf_children(newnode)[0] =
                unsafe_leaf_children(node)[mid + 1];
        }
        if (i < B) {
            copy_elems(offset_elem(elems, i + 1), offset_elem(elems, i),
                       B - 1 - i);
            copy_elems(offset_elem(elems, i), elem, 1);
        } else {
            copy_elems(newelems, offset_elem(elems, B + 1), i - B - 1);
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

static inline
int insert_node(HeightType height,
                leaf_node *node,
                const K *key,
                const V *value,
                struct elem_ref newelem)
{
    int r;
    HeightType h;
    btree_cursor cur;
    h = raw_lookup_node(cur._nodestack, cur._istack, height, node, key);
    if (h != height) {
        leaf_values(cur._nodestack[h])[cur._istack[h]] = *value;
        r = -1;
    } else {
        /* the rest of it does not depend on the comparison operation, only on
           the layout of the structure */
        h = height - 1;
        struct elem_ref elem = {(K *)key, (V *)value, NULL};
        r = insert_node_here(0,
                             cur._nodestack[h],
                             cur._istack[h],
                             elem,
                             newelem);
        while (h-- && r < -1) {
            r = insert_node_here(1,
                                 cur._nodestack[h],
                                 cur._istack[h],
                                 newelem,
                                 newelem);
        }
    }
    return r;
}

static inline
int btree_insert(btree *m, const K *key, const V *value)
{
    if (!m->_root) {
        assert(m->_len == 0);
        assert(m->_height == 0);
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
    K newkey;
    V newvalue;
    leaf_node *newchild;
    struct elem_ref newelem = {&newkey, &newvalue, &newchild};
    int r = insert_node(m->_height,
                        m->_root,
                        key,
                        value,
                        newelem);
    if (r == -1) {                      /* updated an existing element */
        r = 0;
    } else {                            /* added a new element */
        ++m->_len;
    }
    if (r >= 0) {
        return r;
    }
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
    return 0;
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
    parentelem = parent_elem(is_branch, parentnode, previ - 1);
    len = *leaf_len(node);
    lensum = leftlen + len - 1;
    newlen = lensum / 2;
    newleftlen = lensum - newlen;
    dleftlen = leftlen - newleftlen;

    copy_elems(offset_elem(elems, dleftlen + i),
               offset_elem(elems, i + 1),
               len - i - 1);
    copy_elems(offset_elem(elems, dleftlen),
               elems,
               i);
    copy_elems(offset_elem(elems, dleftlen - 1),
               parentelem,
               1);
    copy_elems(elems,
               offset_elem(leftelems, newleftlen + 1),
               dleftlen - 1);
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
    lensum = rightlen + *leaf_len(node) - 1;
    newlen = lensum / 2;
    newrightlen = lensum - newlen;
    drightlen = rightlen - newrightlen;

    copy_elems(offset_elem(elems, i),
               offset_elem(elems, i + 1),
               len - 1 - i);
    copy_elems(offset_elem(elems, len - 1),
               parentelem,
               1);
    copy_elems(offset_elem(elems, len),
               rightelems,
               drightlen - 1);
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
    parentelem = parent_elem(is_branch, parentnode, previ - 1);

    copy_elems(offset_elem(leftelems, leftlen),
               parentelem,
               1);
    copy_elems(offset_elem(leftelems, leftlen + 1),
               elems,
               i);
    copy_elems(offset_elem(leftelems, leftlen + 1 + i),
               offset_elem(elems, i + 1),
               len - i - 1);

    free(node);
    *leaf_len(leftnode) = leftlen + len;
    *oldindex = previ - 1;
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
               len - i - 1);
    copy_elems(offset_elem(elems, len - 1),
               parentelem,
               1);
    copy_elems(offset_elem(elems, len),
               rightelems,
               rightlen);

    free(rightnode);
    *leaf_len(node) = len + rightlen;
    *oldindex = previ;
    return 1;
}

static inline
int cursor_valid(const btree *m, const btree_cursor *cur)
{
    assert(cur->_depth <= m->_height);
    return cur->_depth != m->_height;
}

static inline
void delete_at_cursor(btree *m, btree_cursor *cur)
{
    HeightType depth = cur->_depth;
    HeightType height = m->_height;

    /* a valid cursor cannot belong to a tree of zero height (no elements) */
    assert(height);

    /* iterator must point to an exact match (as opposite to an "in-between"
       match, which is useful only for inserting elements) */
    assert(cursor_valid(m, cur));

    --m->_len;

    /* if node is not leaf, swap with the nearest leaf to the left and fill
       the cursor stacks completely throughout [0, height) */
    if (depth < height - 1) {
        HeightType h = depth;
        leaf_node *uppernode = cur->_nodestack[depth];
        ChildIndexType upperi = cur->_istack[depth];
        K *key = &leaf_keys(uppernode)[upperi];
        leaf_node *node = uppernode;
        ChildIndexType i = upperi;
        ++h;
        do {
            node = branch_children(unsafe_leaf_as_branch(node))[i];
            cur->_nodestack[h] = node;
            i = *leaf_len(node);
            cur->_istack[h] = i;
            ++h;
        } while (h < height);
        --cur->_istack[height - 1];
        leaf_node *lowernode = cur->_nodestack[height - 1];
        ChildIndexType loweri = cur->_istack[height - 1];
        *key = leaf_keys(lowernode)[loweri];
        leaf_values(uppernode)[upperi] = leaf_values(lowernode)[loweri];
        depth = height - 1;
    }

    ChildIndexType i = cur->_istack[depth];
    leaf_node *node = cur->_nodestack[depth];
    while (1) {
        ChildIndexType len = *leaf_len(node);
        int is_branch = depth != height - 1;

        /* easy case: no underflow (or is root node) */
        if (len >= B || !depth) {
            struct elem_ref elem = node_elems(is_branch, node);
            copy_elems(offset_elem(elem, i),
                       offset_elem(elem, i + 1),
                       len - i - 1);
            *leaf_len(node) = len - 1;
            if (!depth && len == 1 && is_branch) {
                /* root node is a branch and is about to become empty */
                m->_root = unsafe_leaf_children(node)[0];
                free(node);
                --m->_height;
            }
            return;
        }

        branch_node *parentnode =
            unsafe_leaf_as_branch(cur->_nodestack[depth - 1]);
        ChildIndexType previ = cur->_istack[depth - 1];

        if (steal_left(is_branch, node, parentnode, i, previ) ||
            steal_right(is_branch, node, parentnode, i, previ))
            break;

        merge_left(is_branch, node, parentnode, i, previ, &i) ||
        merge_right(is_branch, node, parentnode, i, previ, &i);

        --depth;
        node = branch_as_leaf(parentnode);
    }
}

static inline
int btree_delete(btree *m, const K *key)
{
    btree_cursor cur;
    if (!btree_lookup(&cur, m, key)) {
        return 0;
    }
    delete_at_cursor(m, &cur);
    return 1;
}
