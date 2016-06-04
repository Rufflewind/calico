#include <stddef.h>

#include "wclock.h"
static wclock clk;
#ifdef BENCH
static int timing_counter;
static double clk_time;
#define TIME(name)                                                      \
    for (clk_time = get_wclock(&clk), timing_counter = 0;               \
         !timing_counter;                                               \
         ++timing_counter,                                              \
         printf("time_%s=%.6g\n", name, get_wclock(&clk) - clk_time))
#else
#define TIME(name)
#endif

#ifndef B
#define B 8
#endif
#ifndef SearchFunction
#define SearchFunction linear_ordered_search
#endif
#include "linear_ordered_search.h"
#include "binary_search.h"
typedef size_t key_type;
typedef double value_type;

#if 0
#include "btree.h"
cal_def_btree(zd, B, key_type, value_type, 1, cal_pcmp,
              SearchFunction, unsigned short, unsigned char, inline)
#else
#include "btree_head.h"
#define Prefix zd
#define K key_type
#define V value_type
#include "btree_template.h"
#endif

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

#ifdef BENCH
void dummy(void *);
void dummyu(unsigned);
#endif

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
        int r = zd_btree_insert(t, &k, &v);
        value_type *v2;
        assert(!r);
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
}
#else
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
    TIME(name) {
        for (i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            double v = (double)((unsigned)rand() % range);
#ifdef BASE
            dummyu((unsigned)k);
#else
            dummyu((unsigned)zd_btree_insert(t, &k, &v));
#endif
        }
    }
    snprintf(name, sizeof(name), "random_lookups_%u_%u", range, count);
    TIME(name) {
        for (i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
#ifdef BASE
            dummyu((unsigned)k);
#else
            dummyu((unsigned)zd_btree_get(t, &k));
#endif
        }
    }
    printf("len_%u_%u=%zu\n", range, count, zd_btree_len(t));
    snprintf(name, sizeof(name), "sequential_lookups_%u_%u", range, count);
    zd_btree_entry entry;
    TIME(name) {
        for (zd_btree_find_first(t, &entry);
             zd_btree_entry_occupied(t, &entry);
             zd_btree_entry_next(t, &entry)) {
            dummyu((unsigned)*zd_btree_entry_key(&entry) +
                   (unsigned)*zd_btree_entry_get(&entry));
        }
    }
    snprintf(name, sizeof(name), "random_deletes_%u_%u", range, count);
    unsigned ri = 0;
    TIME(name) {
        while (zd_btree_len(t)) {
            size_t k = (unsigned)rand() % range;
            zd_btree_remove(t, &k, NULL, NULL);
            ++ri;
        }
    }
    printf("count_random_deletes_%u_%u=%u\n", range, count, ri);
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
    zd_btree_reset(t);
#else
#ifndef MAX
#define MAX 10000
#endif
    bench_random(t, 80, MAX, MAX);
    zd_btree_reset(t);
#endif
    return 0;
}
