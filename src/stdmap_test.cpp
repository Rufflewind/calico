/*@#bench*/
/*@#depends:black_box.c*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <map>
#define MAP std::map

#include "utils.h"

static
void test_random_inserts(unsigned seed,
                         unsigned range,
                         unsigned count)
{
    MAP<size_t, double> t;
    char name[512];
    srand(seed);
    snprintf(name, sizeof(name), "random_inserts_%u_%u", range, count);
    TIME(name, count) {
        for (unsigned i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            double v = (double)((unsigned)rand() % range);
            t[k] = v;
            black_box(&t[k]);
#ifndef BENCH
            assert(t[k] == v);
#endif
        }
    }
    snprintf(name, sizeof(name), "random_lookups_%u_%u", range, count);
    TIME(name, count) {
        for (unsigned i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            black_box(&t[k]);
        }
    }
}

int main(void)
{
    init_wclock(&clk);
    test_random_inserts(80, 10000, 10000);
}
