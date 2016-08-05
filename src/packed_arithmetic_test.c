#include <assert.h>
#include <stdint.h>
#include "packed_arithmetic.h"

int main(void)
{
    {
        struct cal_i32x2 x = {{12, 34}};
        struct cal_i32x2 y = {{-8, -32}};
        y = cal_add_i32x2(x, y);
        assert(y.array[0] == 4);
        assert(y.array[1] == 2);
        y = cal_sub_i32x2(x, y);
        assert(y.array[0] == 8);
        assert(y.array[1] == 32);
    }
    {
        struct cal_i8x8 x = {{-1, 4, 7, 34, 5, 2, 8, -1}};
        struct cal_i8x8 y = {{-8, -3, 5, 9, 4, 9, -2, -3}};
        y = cal_add_i8x8(x, y);
        assert(y.array[0] == -9);
        assert(y.array[1] == 1);
        assert(y.array[2] == 12);
        assert(y.array[3] == 43);
        assert(y.array[4] == 9);
        assert(y.array[5] == 11);
        assert(y.array[6] == 6);
        assert(y.array[7] == -4);
        y = cal_sub_i8x8(x, y);
        assert(y.array[0] == 8);
        assert(y.array[1] == 3);
        assert(y.array[2] == -5);
        assert(y.array[3] == -9);
        assert(y.array[4] == -4);
        assert(y.array[5] == -9);
        assert(y.array[6] == 2);
        assert(y.array[7] == 3);
    }
    {
        uint64_t x = 0x68860033bdd5b3f4u;
        assert(x == cal_pack_i8x8(cal_unpack_i8x8(x)));
    }
    return 0;
}
