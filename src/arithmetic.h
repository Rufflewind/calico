#ifndef G_O01SKTDA3IE3J0XOJ0SFUII39F91W
#define G_O01SKTDA3IE3J0XOJ0SFUII39F91W
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include "compat/stdint.h"
#include "macros.h"
#include "compat/inline_begin.h"
#ifdef __cplusplus
extern "C" {
#endif
/*@self.public()*/
#include "arithmetic_impl_g.h"
/** @file
    Checked arithmetic functions.  All functions return zero on success, and
    nonzero on failure. */

/** Convert an 8-bit unsigned integer to an 8-bit integer, preserving the
    two's complement representation.  On most platforms this optimizes to a
    no-op.  There is no inverse function for this because casting is safe in
    the other direction. */
static
int8_t cal_u8_to_i8(uint8_t x);

/** Convert a 16-bit unsigned integer to a 16-bit integer, preserving the
    two's complement representation.  On most platforms this optimizes to a
    no-op.  There is no inverse function for this because casting is safe in
    the other direction. */
static
int16_t cal_u16_to_i16(uint16_t x);

/** Convert a 32-bit unsigned integer to a 32-bit integer, preserving the
    two's complement representation.  On most platforms this optimizes to a
    no-op.  There is no inverse function for this because casting is safe in
    the other direction. */
static
int32_t cal_u32_to_i32(uint32_t x);

/** Convert a 64-bit unsigned integer to a 64-bit integer, preserving the
    two's complement representation.  On most platforms this optimizes to a
    no-op.  There is no inverse function for this because casting is safe in
    the other direction. */
static
int64_t cal_u64_to_i64(uint64_t x);

/** Convert an `int` to to a `size_t`.  Returns zero if and only if no
    overflow occurs. */
static
int cal_i_to_z(size_t *y, int x);

#define defun(Bits)                                             \
inline                                                          \
int##Bits##_t cal_u##Bits##_to_i##Bits(uint##Bits##_t x)        \
{                                                               \
    const uint##Bits##_t mid = (uint##Bits##_t)(-1) / 2 + 1;    \
    const int##Bits##_t mmid = (int##Bits##_t)(-1 - (int##Bits##_t)(mid - 1)); \
    if (x >= mid) {                                             \
        return (int##Bits##_t)((int##Bits##_t)(x - mid) + mmid); \
    }                                                           \
    return (int##Bits##_t)x;                                    \
}
defun(8)
defun(16)
defun(32)
defun(64)
#undef defun

inline
int cal_i_to_z(size_t *y, int x)
{
    if (SIZE_MAX <= INT_MAX) {
        if ((int)(size_t)x != x) {
            return 1;
        }
    } else {
        if (x < 0) {
            return 1;
        }
    }
    *y = (size_t)x;
    return 0;
}

#ifdef __cplusplus
}
#endif
#include "compat/inline_end.h"
#endif
