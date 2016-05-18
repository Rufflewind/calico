#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#define MAP std::map

#include "wclock.h"
static wclock clk;
static int timing_counter;
static double clk_time;
#define TIME(name)                                                      \
    for (clk_time = get_wclock(&clk), timing_counter = 0;               \
         !timing_counter;                                               \
         ++timing_counter,                                              \
         printf("time_%s=%.6g\n", name, get_wclock(&clk) - clk_time))

static
void test_random_inserts(unsigned seed,
                         unsigned range,
                         unsigned count,
                         int dump)
{
    MAP<size_t, double> t;
    char name[512];
    srand(seed);
    snprintf(name, sizeof(name), "random_inserts_%u_%u", range, count);
    TIME(name) {
        for (unsigned i = 0; i < count; ++i) {
            size_t k = (unsigned)rand() % range;
            double v = (double)((unsigned)rand() % range);
            t.insert(std::pair<size_t, double>(k, v));
#ifndef PROFILE
            assert(t[k] == v);
#endif
        }
    }
}

int main(void)
{
    init_wclock(&clk);

    test_random_inserts(80, 1000000, 1000000, 0);
}
