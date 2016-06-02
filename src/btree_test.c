#include <stddef.h>
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

// #include "btree.h"
// cal_def_btree(zd, B, key_type, value_type, 1, cal_pcmp,
//               SearchFunction, unsigned short, unsigned char, inline)

#include "btree_head.h"
#define Prefix zd
#define K key_type
#define V value_type
#include "btree_template.h"

static inline
void dump_kv(const key_type *key, const value_type *value)
{
    printf("\033[37m%03zu\033[32m,%03.0f\033[0m\n", *key, *value);
}

static inline
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
                      height - 1,
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
                  height - 1,
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
        dump_node(0, m->_height - 1, m->_root);
    }
    printf("----------------------------------------\n");
}

#include "wclock.h"
static wclock clk;
#ifdef PROFILE
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

#ifdef PROFILE
void dummy(void *);
#else
#define dummy(x) (void)(x)
#endif

static
void test_random_inserts(zd_btree *t,
                         unsigned seed,
                         unsigned range,
                         unsigned count,
                         int dump)
{
    char name[512];
    srand(seed);
    snprintf(name, sizeof(name), "random_inserts_%u_%u", range, count);
    TIME(name) {
        for (unsigned i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            double v = (double)((unsigned)rand() % range);
#ifdef BASE
            dummy(&k);
#else
#ifndef PROFILE
            if (dump) {
                printf("insert(%zu, %f)\n", k, v);
            }
#endif
            int r = zd_btree_insert(t, &k, &v);
            (void)r;
            assert(!r);
#ifndef PROFILE
            if (dump) {
                dump_btree(t);
            }
            value_type *v2 = zd_btree_get(t, &k);
            assert(v2);
            assert(*v2 == v);
#endif
#endif
        }
    }
    snprintf(name, sizeof(name), "random_lookups_%u_%u", range, count);
    TIME(name) {
        for (unsigned i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
#ifdef BASE
            dummy(&k);
#else
// #ifndef PROFILE
//             if (dump) {
//                 printf("lookup(%zu)\n", k);
//             }
// #endif
            dummy(zd_btree_get(t, &k));
#endif
        }
    }
    snprintf(name, sizeof(name), "random_deletes_%u_%u", range, count);
    unsigned ri = 0;
    TIME(name) {
        while (zd_btree_len(t)) {
            size_t k = (unsigned)rand() % range;
#ifndef PROFILE
           // dump_btree(t);
//            printf("delete(%zu), len:%zu\n", k, zd_btree_len(t));
#endif
            zd_btree_remove(t, &k, NULL);
            ++ri;
        }
    }
    printf("count_random_deletes_%u_%u=%u\n", range, count, ri);
    // dump_btree(t);
}

int main(void)
{
    zd_btree bt, *t = &bt;
    init_wclock(&clk);

    zd_btree_init(t);
    zd_btree_reset(t);

    printf("sizeof_leaf_node=%zu\n", sizeof(zdpriv_btree_leaf_node));
    printf("max_height=%zu\n", (size_t)zdpriv_btree_MAX_HEIGHT);
    printf("sizeof_btree_entry=%zu\n", sizeof(zd_btree_entry));

    zd_btree_reset(t);
    test_random_inserts(t, 25, 90, 20, 0);
    zd_btree_reset(t);
    test_random_inserts(t, 80, 10000, 10000, 0);
    zd_btree_reset(t);
    test_random_inserts(t, 100, 100, 300, 0);
    zd_btree_reset(t);
    test_random_inserts(t, 101, 100, 300, 0);
    zd_btree_reset(t);
    test_random_inserts(t, 105, 100, 300, 0);
    zd_btree_reset(t);
    test_random_inserts(t, 1, 40, 40, 0);
    zd_btree_reset(t);
    return 0;
}
