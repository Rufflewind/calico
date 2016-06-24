#include <stdio.h>
#include <stdlib.h>
#include "frozen_btree.h"
#include "macros.h"

typedef long Type;

static inline
int cmp(void *ctx, const void *x, const void *y)
{
    (void)ctx;
    return cal_cmp(NULL, *(const Type *)x, *(const Type *)y);
}

void simple_test_b(void)
{
    size_t pos;
    long key;
    const long array[] = {
        3, 7, 11,

        -1, 1, 2,
        4, 5, 6,
        8, 9, 10,
        12, 13, 14,

        -3, -2
    };
    for (key = -3; key < 16; ++key) {
        int r = cal_fbt_search_b(&key,
                                 array,
                                 sizeof(array) / sizeof(*array),
                                 sizeof(*array),
                                 &cmp,
                                 NULL,
                                 4,
                                 &pos);
        assert(!!r == (key >= -3 && key <= 14 && key));
        // printf("key=%li,r=%i\n", key, r);
        if (r) {
            // printf("pos=%zu\n", pos);
            assert(array[pos] == key);
        }
    }
}

void simple_test_l(void)
{
    size_t pos;
    long key;
    const long array[] = {
        3, 7, 11,

        -1, 1, 2,
        4, 5, 6,
        8, 9, 10,
        12, 13, 14,

        -3, -2
    };
    for (key = -3; key < 16; ++key) {
        int r = cal_fbt_search_l(&key,
                                 array,
                                 sizeof(array) / sizeof(*array),
                                 sizeof(*array),
                                 &cmp,
                                 NULL,
                                 4,
                                 &pos);
        assert(!!r == (key >= -3 && key <= 14 && key));
        // printf("key=%li,r=%i\n", key, r);
        if (r) {
            // printf("pos=%zu\n", pos);
            assert(array[pos] == key);
        }
    }
}

void long_test(size_t b)
{
    size_t ns[] = {
        0, 1, 2, 3, 4, 5,
        10, 20, 50, 100, 200, 500, 1000
    };
    size_t ni;
    for (ni = 0; ni < sizeof(ns) / sizeof(*ns); ++ni) {
        size_t i;
        size_t n = ns[ni];
        Type key;
        Type *array = malloc(n * sizeof(*array));
        Type *tree = malloc(n * sizeof(*tree));
        for (i = 0; i < n; ++i) {
            array[i] = (Type)(i * 2);
        }

        // printf("n=%zu\n", n);
        // printf("array:");
        // for (i = 0; i < n; ++i) {
        //     printf(" %li", array[i]);
        // }
        // printf("\n");

        cal_fbt_build(tree, array, n, sizeof(*array), b);

        // printf("tree: ");
        // for (i = 0; i < n; ++i) {
        //     printf(" %li", tree[i]);
        // }
        // printf("\n");

        for (key = -1; key < (Type)(2 * n - 1); ++key) {
            int r = cal_fbt_search_l(&key,
                                     tree,
                                     n,
                                     sizeof(*tree),
                                     &cmp,
                                     NULL,
                                     b,
                                     &i);
//            printf("search: key=%li, r=%i\n", key, r);
            assert(!!r == !(key % 2));  // only even keys exist
            if (r) {
                assert(tree[i] == key);
//                printf("tree[%zu]=%li\n", i, tree[i]);
            }
        }
        free(tree);
        free(array);
    }
}

int main(void)
{
    simple_test_b();
    simple_test_l();
    long_test(3);
    return 0;
}
