#include <assert.h>
#include <stdint.h>
#define Type(x) int64_t x
#define KeysPerNode 8
#include "btree.inl"
#include "bsearch_Type.h"

int test_bsearch(int64_t **pptr,
                 size_t count,
                 const int64_t *key,
                 void *cmp_ctx)
{
    return rf_bsearch_int64_t(key, pptr, count, cmp_ctx);
}

int main(void)
{
    Type(a[]) = {2, 3, 6, 8, 12};

    Type(*p);
    Type(k);

    p = a;
    k = 0;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 0);

    p = a;
    k = 2;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 0);

    p = a;
    k = 3;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 1);

    p = a;
    k = 6;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 2);

    p = a;
    k = 7;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 3);

    p = a;
    k = 8;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 3);

    p = a;
    k = 9;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 4);

    p = a;
    k = 12;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 4);

    p = a;
    k = 14;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k, NULL));
    assert(p - a == 5);

    return 0;
}
