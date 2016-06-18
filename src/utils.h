#ifndef G_H5607C32N66ZQ6KQ8RIZKVWKQ1BB5
#define G_H5607C32N66ZQ6KQ8RIZKVWKQ1BB5
#ifdef __cplusplus
extern "C" {
#endif

#include "wclock.h"
static wclock clk;
#ifdef BENCH
static int timing_counter;
static double clk_time;
#define TIME(name, repeats)                                             \
    for (clk_time = get_wclock(&clk), timing_counter = 0;               \
         !timing_counter;                                               \
         ++timing_counter,                                              \
         printf("time_%s=%.6g\n", name,                                 \
                (get_wclock(&clk) - clk_time) / repeats))

/* black boxes to prevent optimizations */
void black_box(void *);
void black_box_u(unsigned);

#else

#define TIME(name, repeats)

#define black_box(x) (void)(x)
#define black_box_u(x) (void)(x)

#endif

#ifdef BENCH
#endif

#ifdef __cplusplus
}
#endif
#endif
