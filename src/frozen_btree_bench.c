/*@self.depends(["black_box.c"])*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "frozen_btree.h"
#include "macros.h"
#define BENCH
#include "utils.h"

#ifndef REPEATS
#define REPEATS 1000
#endif
#ifndef B
#define B 4
#endif

static inline
int cmp(void *ctx, const void *x, const void *y)
{
    (void)ctx;
    return cal_cmp(NULL, *(const size_t *)x, *(const size_t *)y);
}

static
void bench_lookup(unsigned seed, unsigned count)
{
    char name[512];
    unsigned i, dummy = 0;
    size_t *array = malloc(count * sizeof(*array));
    size_t *tree = malloc(count * sizeof(*tree));
    size_t j;

    for (i = 0; i < count; ++i) {
        array[i] = i * 2 + 1;
    }
    cal_fbt_build(tree, array, count, sizeof(*array), B);

    srand(seed);

    snprintf(name, sizeof(name), "lookup_l_sequential_%u", count);
    j = 0;
    TIME(name, REPEATS) {
        for (i = 0; i < REPEATS; ++i) {
            size_t r;
            if (cal_fbt_search_l(&j, tree, count, sizeof(*tree),
                                 &cmp, NULL, B, &r)) {
                dummy += r;
            }
            ++j;
        }
    }

    snprintf(name, sizeof(name), "lookup_b_sequential_%u", count);
    j = 0;
    TIME(name, REPEATS) {
        for (i = 0; i < REPEATS; ++i) {
            size_t r;
            if (cal_fbt_search_b(&j, tree, count, sizeof(*tree),
                                 &cmp, NULL, B, &r)) {
                dummy += r;
            }
            ++j;
        }
    }

    snprintf(name, sizeof(name), "lookup_l_random_%u", count);
    TIME(name, REPEATS) {
        for (i = 0; i < REPEATS; ++i) {
            size_t k = (unsigned)rand() % (2 * count);
            size_t r;
            if (cal_fbt_search_l(&k, tree, count, sizeof(*tree),
                                 &cmp, NULL, B, &r)) {
                dummy += r;
            }
        }
    }

    snprintf(name, sizeof(name), "lookup_b_random_%u", count);
    TIME(name, REPEATS) {
        for (i = 0; i < REPEATS; ++i) {
            size_t k = (unsigned)rand() % (2 * count);
            size_t r;
            if (cal_fbt_search_b(&k, tree, count, sizeof(*tree),
                                 &cmp, NULL, B, &r)) {
                dummy += r;
            }
        }
    }

    free(array);
    free(tree);
    black_box_u(dummy);
}

int main(void)
{
    init_wclock(&clk);
    bench_lookup(1, 100);
    bench_lookup(1, 1000);
    bench_lookup(1, 10000);
    return 0;
}
