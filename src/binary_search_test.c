#include <assert.h>
#include <stdint.h>
#include "binary_search.h"
#define Type int64_t

static inline
int compare_Type(const Type *x, const Type *y)
{
    return (*x > *y) - (*x < *y);
}

static inline
int generic_compare_Type(void *ctx, const void *x, const void *y)
{
    (void)ctx;
    return compare_Type((const Type *)x, (const Type *)y);
}

static inline
int binary_search_Type(const Type *key,
                       const Type *ptr,
                       size_t count,
                       size_t *pos_out)
{
    return binary_search(key, ptr, count, sizeof(Type),
                         &generic_compare_Type, NULL, pos_out);
}

static inline
int test_bsearch(Type **pptr, size_t count, const Type *key)
{
    size_t pos;
    int r = binary_search_Type(key, *pptr, count, &pos);
    *pptr = *pptr + pos;
    return r;
}

int main(void)
{
    Type a[] = {2, 3, 6, 8, 12};
    Type *p;
    Type k;

    p = NULL;
    k = 0;
    assert(!test_bsearch(&p, 0, &k));
    assert(!p);

    p = a;
    k = 0;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 0);

    p = a;
    k = 2;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 0);

    p = a;
    k = 3;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 1);

    p = a;
    k = 6;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 2);

    p = a;
    k = 7;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 3);

    p = a;
    k = 8;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 3);

    p = a;
    k = 9;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 4);

    p = a;
    k = 12;
    assert(test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 4);

    p = a;
    k = 14;
    assert(!test_bsearch(&p, sizeof(a) / sizeof(*a), &k));
    assert(p - a == 5);

    return 0;
}
