#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "shuffle.h"

#define N 100

static
size_t randfunc(void *ctx, size_t max)
{
    assert(!ctx);
    return (size_t)rand() % (max + 1);
}

int main(void)
{
    long i, array[N], found[N] = {0};
    for (i = 0; i < N; ++i) {
        array[i] = i;
    }
    cal_shuffle(array, N, sizeof(*array), &i, &randfunc, NULL);
    for (i = 0; i < N; ++i) {
        ++found[array[i]];
        printf(" %li", array[i]);
    }
    printf("\n");
    for (i = 0; i < N; ++i) {
        assert(found[i] == 1);
    }
    return 0;
}
