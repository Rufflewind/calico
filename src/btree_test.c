#include <stddef.h>
#ifndef B
#define B 8
#endif
#define K size_t
#define V double
#ifndef LOOKUP_METHOD
#define LOOKUP_METHOD linear_sorted_search_K_V
#endif
static inline
int compare_K(const K *x, const K *y)
{
    return (*x > *y) - (*x < *y);
}
static inline
int generic_compare_K(void *ctx, const void *x, const void *y)
{
    (void)ctx;
    return compare_K((const K *)x, (const K *)y);
}

#include "btree.c"

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
void test_random_inserts(btree *t,
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
            int r = btree_insert(t, &k, &v);
            (void)r;
            assert(!r);
#ifndef PROFILE
            if (dump) {
                dump_btree(t);
            }
            assert(*btree_get(t, &k) == v);
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
            dummy(btree_get(t, &k));
#endif
        }
    }
    snprintf(name, sizeof(name), "random_lookups_%u_%u", range, count);
    unsigned ri = 0;
    TIME(name) {
        while (btree_len(t)) {
            size_t k = (unsigned)rand() % range;
#ifdef BASE
            dummy(&k);
#else
            btree_cursor cur;
#ifndef PROFILE
//            dump_btree(t);
            printf("delete(%zu)\n", k);
#endif
            if (btree_lookup(&cur, t, &k)) {
                delete_at_cursor(t, &cur);
            }
#endif
            ++ri;
        }
    }
#ifndef PROFILE
    dump_btree(t);
#endif
}

int main(void)
{
    btree bt, *t = &bt;
    init_wclock(&clk);

    init_btree(t);
    reset_btree(t);

#define ITERATOR_HEIGHT (UINTPTR_MAX / sizeof(leaf_node))

    printf("sizeof_leaf_node=%zu\n", sizeof(leaf_node));
    printf("max_height=%zu\n", MAX_HEIGHT);

    reset_btree(t);
    test_random_inserts(t, 25, 90, 90, 0);
    reset_btree(t);
    test_random_inserts(t, 80, 10000, 10000, 0);
    reset_btree(t);
    test_random_inserts(t, 100, 100, 300, 0);
    reset_btree(t);
    test_random_inserts(t, 101, 100, 300, 0);
    reset_btree(t);
    test_random_inserts(t, 105, 100, 300, 0);
    reset_btree(t);

    test_random_inserts(t, 1, 40, 40, 0);
    // btree_cursor cur;
    // K k;

    // k = 17;
    // btree_lookup(&cur, t, &k);
    // delete_at_cursor(t, &cur);

    // k = 21;
    // btree_lookup(&cur, t, &k);
    // delete_at_cursor(t, &cur);

    // k = 23;
    // btree_lookup(&cur, t, &k);
    // delete_at_cursor(t, &cur);

    // dump_btree(t);

    // k = 24;
    // btree_lookup(&cur, t, &k);
    // delete_at_cursor(t, &cur);

    // dump_btree(t);

    reset_btree(t);
    return 0;
}
