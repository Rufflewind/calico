#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "arithmetic.h"

void test_conversions(void)
{

#define test(func, signed_type, value)                          \
    assert((signed_type)(value) ==                              \
           func((u ## signed_type)(signed_type)(value)));
    test(cal_u8_to_i8, int8_t, 42);
    test(cal_u8_to_i8, int8_t, -42);
    test(cal_u16_to_i16, int16_t, 42);
    test(cal_u16_to_i16, int16_t, -42);
    test(cal_u32_to_i32, int32_t, 42);
    test(cal_u32_to_i32, int32_t, -42);
    test(cal_u64_to_i64, int64_t, 42);
    test(cal_u64_to_i64, int64_t, -42);
#undef test

    size_t n;
    int i;
    assert(!cal_i_to_z(&n, 12));
    assert(n == (size_t)12);
    assert(cal_i_to_z(&n, -1));
    assert(!cal_add_i(&i, 1, 2));
    assert(cal_add_i(&i, 1, INT_MAX));
}

int main(void)
{
    test_conversions();
    return 0;
}
