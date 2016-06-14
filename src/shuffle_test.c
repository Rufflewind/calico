#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "shuffle.h"

#define N 100

int main(void)
{
    long i, array[N], found[N] = {0};
    for (i = 0; i < N; ++i) {
        array[i] = i;
    }
    cal_shuffle(array, N, sizeof(*array), &i);
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
