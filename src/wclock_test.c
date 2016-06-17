#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "wclock.h"
#ifdef _WIN32
# include <windows.h>
# define sleep Sleep
#else
# include <unistd.h>
#endif

int main(void)
{
    double res, t1, t2;
    wclock clock;

    if (init_wclock(&clock)) {
        abort();
    }

    res = get_wclock_res(&clock);
    printf("%.17g\n", res);
    assert(res > 0);
    assert(res < 2e-3);  /* presumably the clock has at least ms precision! */

    t1 = get_wclock(&clock);
    printf("%.17g\n", t1);

    sleep(1);

    t2 = get_wclock(&clock);
    printf("%.17g\n", t2);
    printf("%.17g\n", t2 - t1);
    assert(fabs(t2 - t1 - 1.) < 1e-1);

    return 0;
}
