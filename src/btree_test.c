/*@#bench*/
/*@#depends:black_box.c*/
#include <stddef.h>
#include "utils.h"

#ifndef MAX
#define MAX 10000
#endif

#ifndef MinArity
#define MinArity 8
#endif
#ifndef SearchFunction
#define SearchFunction cal_linear_ordered_search
#endif
#include "linear_ordered_search.h"
#include "binary_search.h"
typedef size_t key_type;
typedef double value_type;

#if 0
#include "btree.h"
cal_def_btree(zd, MinArity, key_type, value_type, 1, cal_pcmp,
              SearchFunction, unsigned short, unsigned char, inline)
#else
#include "btree_head.h"
#define Prefix zd
#define KeyType key_type
#define ValueType value_type
#include "btree_template.h"
#endif

#include "btree_head.h"
#define Prefix zz
#define KeyType size_t
#define ValueType size_t
#include "btree_template.h"

#include "btree_head.h"
#define Prefix z
#define KeyType size_t
#include "btree_template.h"

#include "shuffle.h"

#include "compat/inline_begin.h"

static
void dump_kv(const key_type *key, const value_type *value)
{
    printf("\033[37m%03zu\033[32m,%03.0f\033[0m\n", *key, *value);
}

static
void dump_node(size_t indent, unsigned char height, zdpriv_btree_leaf_node *m)
{
    static const char *indent_str = "  ";
    size_t j;
    unsigned short i;
    zdpriv_btree_branch_node *mb = zdpriv_btree_try_leaf_as_branch(height, m);
#ifdef VERBOSE
    for (j = 0; j < indent; ++j) {
        printf("%s", indent_str);
    }
    printf("dump_node(%p)\n", (void *)m);
#endif
    for (i = 0; i < *zdpriv_btree_leaf_len(m); ++i) {
        if (mb) {
            dump_node(indent + 1,
                      (unsigned char)(height - 1),
                      zdpriv_btree_branch_children(mb)[i]);
        }
        for (j = 0; j < indent; ++j) {
            printf("%s", indent_str);
        }
        dump_kv(zdpriv_btree_leaf_keys(m) + i,
                zdpriv_btree_leaf_values(m) + i);
    }
    if (mb) {
        dump_node(indent + 1,
                  (unsigned char)(height - 1),
                  zdpriv_btree_branch_children(mb)[i]);
    }
}

/** For debugging purposes. */
static inline CAL_UNUSED
void dump_btree(zd_btree *m)
{
    if (!m->_root) {
        printf("(no root node)\n");
    } else {
        dump_node(0, (unsigned char)(m->_height - 1), m->_root);
    }
    printf("----------------------------------------\n");
}

#ifndef BENCH

static
void test_random(zd_btree *t,
                 unsigned seed,
                 unsigned range,
                 unsigned count)
{
    unsigned i, ri;
    size_t l;
    key_type *keys;
    long prev_key;
    zd_btree_entry entry;

    srand(seed);
    for (i = 0; i < count; ++i) {
        size_t k = (unsigned)rand() % range;
        double v = (double)(range - k);
        int r = zd_btree_insert(t, &k, &v, NULL);
        value_type *v2;
        assert(r >= 0);
        v2 = zd_btree_get(t, &k);
        assert(v2);
        assert(*v2 == v);
    }
    for (i = 0; i < count; ++i) {
        size_t k = (unsigned)rand() % range;
        zd_btree_get(t, &k);
    }
    l = zd_btree_len(t);

    keys = (size_t *)malloc(l * sizeof(*keys));
    prev_key = -1;
    for (zd_btree_find_first(t, &entry);
         zd_btree_entry_occupied(t, &entry);
         zd_btree_entry_next(t, &entry)) {
        key_type key = *zd_btree_entry_key(&entry);
        value_type value = *zd_btree_entry_get(&entry);
        assert(prev_key < (long)key);
        prev_key = (long)key;
        assert(range - key == value);
        --l;
        keys[l] = key;
    }
    assert(l == 0);
    for (zd_btree_find_last(t, &entry);
         zd_btree_entry_occupied(t, &entry);
         zd_btree_entry_prev(t, &entry)) {
        key_type key = *zd_btree_entry_key(&entry);
        value_type value = *zd_btree_entry_get(&entry);
        assert(keys[l] == key);
        assert(range - key == value);
        ++l;
    }
    assert(l == zd_btree_len(t));
    free(keys);

    ri = 0;
    while (zd_btree_len(t)) {
        size_t k = (unsigned)rand() % range;
        zd_btree_remove(t, &k, NULL, NULL);
        ++ri;
    }

    /* regression test for a bug that occurs when the root node is empty */
    zd_btree_find_first(t, &entry);
    assert(!zd_btree_entry_occupied(t, &entry));
    zd_btree_find_last(t, &entry);
    assert(!zd_btree_entry_occupied(t, &entry));
}

#else /* benchmarking code below */

static
void bench_random(zd_btree *t,
                  unsigned seed,
                  unsigned range,
                  unsigned count)
{
    char name[512];
    unsigned i;
    srand(seed);
    snprintf(name, sizeof(name), "random_inserts_%u_%u", range, count);
    TIME(name, count) {
        for (i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            double v = (double)((unsigned)rand() % range);
#ifdef BASE
            black_box_u((unsigned)k);
#else
            black_box_u((unsigned)zd_btree_insert(t, &k, &v, NULL));
#endif
        }
    }
    snprintf(name, sizeof(name), "random_lookups_%u_%u", range, count);
    TIME(name, count) {
        for (i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
#ifdef BASE
            black_box_u((unsigned)k);
#else
            unsigned char u;
            void *ptr = zd_btree_get(t, &k);
            memcpy(&u, &ptr, 1);
            black_box_u(u);
#endif
        }
    }
    printf("len_%u_%u=%zu\n", range, count, zd_btree_len(t));
    snprintf(name, sizeof(name), "sequential_lookups_%u_%u", range, count);
    zd_btree_entry entry;
    TIME(name, zd_btree_len(t)) {
        for (zd_btree_find_first(t, &entry);
             zd_btree_entry_occupied(t, &entry);
             zd_btree_entry_next(t, &entry)) {
            black_box_u((unsigned)*zd_btree_entry_key(&entry) +
                        (unsigned)*zd_btree_entry_get(&entry));
        }
    }
    snprintf(name, sizeof(name), "random_deletes_%u_%u", range, count);
    unsigned ri = 0;
    TIME(name, ri) {
        while (zd_btree_len(t)) {
            size_t k = (unsigned)rand() % range;
            zd_btree_remove(t, &k, NULL, NULL);
            ++ri;
        }
    }
    printf("count_random_deletes_%u_%u=%u\n", range, count, ri);
}

void bench_random_ir(unsigned seed, unsigned count)
{
    char name[512];
    zz_btree bt, *t = &bt;
    unsigned i;

    srand(seed);
    zz_btree_init(t);

    snprintf(name, sizeof(name), "random_ir_ins_%u", count);
    TIME(name, count) {
        for (i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % count;
            zz_btree_insert(t, &k, &k, NULL);
        }
    }

    snprintf(name, sizeof(name), "random_ir_%u", count);
    TIME(name, 1000000) {
        for (i = 0; i < 1000000; ++i) {
            size_t k = (unsigned)rand() % count;
            zz_btree_insert(t, &k, &k, NULL);
            zz_btree_remove(t, &k, NULL, NULL);
        }
    }

    black_box(t);
}

void bench_sequential_ir(unsigned count)
{
    char name[512];
    zz_btree bt, *t = &bt;
    unsigned i;

    zz_btree_init(t);

    snprintf(name, sizeof(name), "sequential_ir_ins_%u", count);
    TIME(name, count) {
        for (i = 0; i < count; ++i) {
            size_t k = i * 2;
            zz_btree_insert(t, &k, &k, NULL);
        }
    }

    snprintf(name, sizeof(name), "sequential_ir_%u", count);
    TIME(name, 1000000) {
        size_t k = 1;
        for (i = 0; i < 1000000; ++i) {
            zz_btree_insert(t, &k, &k, NULL);
            zz_btree_remove(t, &k, NULL, NULL);
            k = (k + 2) % count;
        }
    }

    black_box(t);
}
#endif

int main(void)
{
    zd_btree bt, *t = &bt;
    init_wclock(&clk);

    zd_btree_init(t);
    zd_btree_reset(t);
    zd_btree_reset(t);

#ifndef BENCH

    printf("sizeof_leaf_node=%zu\n", sizeof(zdpriv_btree_leaf_node));
    printf("max_height=%zu\n", (size_t)zdpriv_btree_MAX_HEIGHT);
    printf("sizeof_btree_entry=%zu\n", sizeof(zd_btree_entry));

    zd_btree_reset(t);
    test_random(t, 25, 90, 20);
    zd_btree_reset(t);
    test_random(t, 80, 10000, 10000);
    zd_btree_reset(t);
    test_random(t, 100, 100, 300);
    zd_btree_reset(t);
    test_random(t, 101, 100, 300);
    zd_btree_reset(t);
    test_random(t, 105, 100, 300);
    zd_btree_reset(t);
    test_random(t, 1, 40, 40);

#else

    bench_random_ir(1, 100);
    bench_random_ir(2, 10000);
    bench_sequential_ir(100);
    bench_sequential_ir(10000);
    bench_random(t, 80, MAX, MAX);

#endif

    zd_btree_reset(t);
    return 0;
}
