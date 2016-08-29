#ifndef G_ED64P8RGJN8XDUAT6RENZ7I7LS2EP
#define G_ED64P8RGJN8XDUAT6RENZ7I7LS2EP
#include <stdint.h>
#ifdef __SSE2__
# include <emmintrin.h>
#endif
#include "arithmetic.h"
#define CAL_ALIGNAS_OPTIONAL
#include "compat/alignas_begin.h"
#undef CAL_ALIGNAS_OPTIONAL
#include "compat/inline_begin.h"
#ifdef __cplusplus
extern "C" {
#endif
/*@self.public()*/
/** @file
    Packed arithmetic functions.  The functions will use SSE intrinsics where
    available. */

/** A pack of two 32-bit integers. */
struct cal_i32x2 {
    /** Entries of the pack. */
    alignas(8) int32_t array[2];
};

/** An initializer macro with all entries zero. */
#define CAL_I32X2_INITIALIZER {{(int32_t)0, (int32_t)0}}

/** A constant value with all entries zero. */
static
const struct cal_i32x2 CAL_I32X2_ZERO = CAL_I32X2_INITIALIZER;

/** Pack the bits of an `i32x2` into a 64-bit unsigned integer.  The result is
    always ittle-endian regardless of platform. */
static
uint64_t cal_i32x2_pack(struct cal_i32x2 x);

/** Inverse of `cal_i32x2_pack`. */
static
struct cal_i32x2 cal_i32x2_unpack(uint64_t x);

/** Add packed integers. */
static
struct cal_i32x2 cal_i32x2_add(struct cal_i32x2 x, struct cal_i32x2 y);

/** Subtract packed integers. */
static
struct cal_i32x2 cal_i32x2_sub(struct cal_i32x2 x, struct cal_i32x2 y);

/** A pack of four 8-bit integers. */
struct cal_i8x4 {
    /** Entries of the pack. */
    alignas(4) int8_t array[4];
};

/** An initializer macro with all entries zero. */
#define CAL_I8X4_INITIALIZER {{(int8_t)0, (int8_t)0, (int8_t)0, (int8_t)0}}

/** A constant value with all entries zero. */
static
const struct cal_i8x4 CAL_I8X4_ZERO = CAL_I8X4_INITIALIZER;

/** Pack the bits of an `i8x4` into a 32-bit unsigned integer.  The result is
    always little-endian regardless of platform. */
static
uint32_t cal_i8x4_pack(struct cal_i8x4 t);

/** Inverse of `cal_i8x4_pack`. */
static
struct cal_i8x4 cal_i8x4_unpack(uint32_t i);

/** Add packed integers. */
static
struct cal_i8x4 cal_i8x4_add(struct cal_i8x4 x, struct cal_i8x4 y);

/** Subtract packed integers. */
static
struct cal_i8x4 cal_i8x4_sub(struct cal_i8x4 x, struct cal_i8x4 y);

/** A pack of eight 8-bit integers. */
struct cal_i8x8 {
    /** Entries of the pack. */
    alignas(8) int8_t array[8];
};

/** An initializer macro with all entries zero. */
#define CAL_I8X8_INITIALIZER {{(int8_t)0, (int8_t)0, (int8_t)0, (int8_t)0, \
                               (int8_t)0, (int8_t)0, (int8_t)0, (int8_t)0}}

/** A constant value with all entries zero. */
static
const struct cal_i8x8 CAL_I8X8_ZERO = CAL_I8X8_INITIALIZER;

/** Pack the bits of an `i8x8` into a 64-bit unsigned integer.  The result is
    always little-endian regardless of platform. */
static
uint64_t cal_i8x8_pack(struct cal_i8x8 x);

/** Inverse of `cal_i8x8_pack`. */
static
struct cal_i8x8 cal_i8x8_unpack(uint64_t x);

/** Add packed integers. */
static
struct cal_i8x8 cal_i8x8_add(struct cal_i8x8 x, struct cal_i8x8 y);

/** Subtract packed integers. */
static
struct cal_i8x8 cal_i8x8_sub(struct cal_i8x8 x, struct cal_i8x8 y);

/** Combine two `cal_i8x4` into a `cal_i8x8`.

    @param x0 Entries 0 to 3.
    @param x1 Entries 4 to 7.

*/
static
struct cal_i8x8 cal_i8x8_from_i8x4(struct cal_i8x4 x0, struct cal_i8x4 x1);

/** Truncate a `cal_i8x8` to a `cal_i8x4`.  Entries 4 to 7 are discarded. */
static
struct cal_i8x4 cal_i8x8_lower_i8x4(struct cal_i8x8 x);

inline
uint64_t cal_i32x2_pack(struct cal_i32x2 x)
{
    return
        (uint64_t)(uint32_t)x.array[0] << 0 |
        (uint64_t)(uint32_t)x.array[1] << 32;
}

inline
struct cal_i32x2 cal_i32x2_unpack(uint64_t x)
{
    /* see comment in cal_i8x8_unpack for an explanation of why we do this */
#if defined __GNUC__ &&                                                     \
    defined __BYTE_ORDER__ &&                                               \
    defined __ORDER_LITTLE_ENDIAN__ &&                                      \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    union {
        struct cal_i32x2 r;
        uint64_t i;
    } u;
    u.i = x;
    return u.r;
#else
    struct cal_i32x2 r = {{
        cal_u32_to_i32((uint32_t)(x >> 0)),
        cal_u32_to_i32((uint32_t)(x >> 32))
    }};
    return r;
#endif
}

inline
struct cal_i32x2 cal_i32x2_add(struct cal_i32x2 x, struct cal_i32x2 y)
{
    /* as explained in cal_i8x8_add, we need to use intrinsics here */
#ifdef __SSE2__
    return cal_i32x2_unpack(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_add_epi32(
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i32x2_pack(x))),
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i32x2_pack(y)))
            )
        )
    );
#else
    size_t i;
    struct cal_i32x2 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] + y.array[i];
    }
    return z;
#endif
}

inline
struct cal_i32x2 cal_i32x2_sub(struct cal_i32x2 x, struct cal_i32x2 y)
{
    /* as explained in cal_i8x8_add, we need to use intrinsics here */
#ifdef __SSE2__
    return cal_i32x2_unpack(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_sub_epi32(
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i32x2_pack(x))),
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i32x2_pack(y)))
            )
        )
    );
#else
    size_t i;
    struct cal_i32x2 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] - y.array[i];
    }
    return z;
#endif
}

inline
uint32_t cal_i8x4_pack(struct cal_i8x4 t)
{
    /* this should optimize to a simple mov */
    return
        (uint32_t)(uint8_t)t.array[0] << 0 |
        (uint32_t)(uint8_t)t.array[1] << 8 |
        (uint32_t)(uint8_t)t.array[2] << 16 |
        (uint32_t)(uint8_t)t.array[3] << 24;
}

inline
struct cal_i8x4 cal_i8x4_unpack(uint32_t i)
{
    /* see comment in cal_i8x8_unpack for an explanation of why we do this */
#if defined __GNUC__ &&                                                     \
    defined __BYTE_ORDER__ &&                                               \
    defined __ORDER_LITTLE_ENDIAN__ &&                                      \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    union {
        struct cal_i8x4 r;
        uint32_t i;
    } u;
    u.i = i;
    return u.r;
#else
    struct cal_i8x4 r = {{
        cal_u8_to_i8((uint8_t)(i >> 0)),
        cal_u8_to_i8((uint8_t)(i >> 8)),
        cal_u8_to_i8((uint8_t)(i >> 16)),
        cal_u8_to_i8((uint8_t)(i >> 24))
    }};
    return r;
#endif
}

inline
struct cal_i8x4 cal_i8x4_add(struct cal_i8x4 x, struct cal_i8x4 y)
{
    /* adding them as packed integers is twice as fast as the naive approach */
    return cal_i8x8_lower_i8x4(
        cal_i8x8_add(
            cal_i8x8_from_i8x4(x, CAL_I8X4_ZERO),
            cal_i8x8_from_i8x4(y, CAL_I8X4_ZERO)
        )
    );
}

inline
struct cal_i8x4 cal_i8x4_sub(struct cal_i8x4 x, struct cal_i8x4 y)
{
    /* see comment in cal_i8x4_add for why we do this */
    return cal_i8x8_lower_i8x4(
        cal_i8x8_sub(
            cal_i8x8_from_i8x4(x, CAL_I8X4_ZERO),
            cal_i8x8_from_i8x4(y, CAL_I8X4_ZERO)
        )
    );
}

inline
uint64_t cal_i8x8_pack(struct cal_i8x8 x)
{
    return
        (uint64_t)(uint8_t)x.array[0] << 0 |
        (uint64_t)(uint8_t)x.array[1] << 8 |
        (uint64_t)(uint8_t)x.array[2] << 16 |
        (uint64_t)(uint8_t)x.array[3] << 24 |
        (uint64_t)(uint8_t)x.array[4] << 32 |
        (uint64_t)(uint8_t)x.array[5] << 40 |
        (uint64_t)(uint8_t)x.array[6] << 48 |
        (uint64_t)(uint8_t)x.array[7] << 56;
}

inline
struct cal_i8x8 cal_i8x8_unpack(uint64_t x)
{
    /*
      (1) Unlike Clang 3.7, GCC 5.3 doesn't know how to optimize the
          bit-shifts.

      (2) memcpy is one option, but testing shows GCC is somewhat better at
          optimizing unions (and they are equivalent anyway)

      (3) GCC bug #69871 indicates that alignment information of arguments
          can sometimes get lost during inlining, so enforcing the alignment
          on the type itself is important.  Also, using an array instead of
          named members can help.
    */
#if defined __GNUC__ &&                                                     \
    defined __BYTE_ORDER__ &&                                               \
    defined __ORDER_LITTLE_ENDIAN__ &&                                      \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    union {
        struct cal_i8x8 r;
        uint64_t i;
    } u;
    u.i = x;
    return u.r;
#else
    struct cal_i8x8 r = {{
        cal_u8_to_i8((uint8_t)(x >> 0)),
        cal_u8_to_i8((uint8_t)(x >> 8)),
        cal_u8_to_i8((uint8_t)(x >> 16)),
        cal_u8_to_i8((uint8_t)(x >> 24)),
        cal_u8_to_i8((uint8_t)(x >> 32)),
        cal_u8_to_i8((uint8_t)(x >> 40)),
        cal_u8_to_i8((uint8_t)(x >> 48)),
        cal_u8_to_i8((uint8_t)(x >> 56))
    }};
    return r;
#endif
}

inline
struct cal_i8x8 cal_i8x8_add(struct cal_i8x8 x, struct cal_i8x8 y)
{
    /* unlike ICC 13.0, Clang 3.7 and GCC 5.3 weren't able to optimize the
       naive loop into a simple instruction */
#ifdef __SSE2__
    return cal_i8x8_unpack(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_add_epi8(
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i8x8_pack(x))),
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i8x8_pack(y)))
            )
        )
    );
#else
    size_t i;
    struct cal_i8x8 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] + y.array[i];
    }
    return z;
#endif
}

inline
struct cal_i8x8 cal_i8x8_sub(struct cal_i8x8 x, struct cal_i8x8 y)
{
    /* as explained in cal_i8x8_add, we need to use intrinsics here */
#ifdef __SSE2__
    return cal_i8x8_unpack(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_sub_epi8(
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i8x8_pack(x))),
                _mm_cvtsi64_si128(cal_u64_to_i64(cal_i8x8_pack(y)))
            )
        )
    );
#else
    size_t i;
    struct cal_i8x8 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] - y.array[i];
    }
    return z;
#endif
}

inline
struct cal_i8x8 cal_i8x8_from_i8x4(struct cal_i8x4 x0, struct cal_i8x4 x1)
{
    return cal_i8x8_unpack(
        (uint64_t)cal_i8x4_pack(x0) << 0 |
        (uint64_t)cal_i8x4_pack(x1) << 32
    );
}

inline
struct cal_i8x4 cal_i8x8_lower_i8x4(struct cal_i8x8 x)
{
    return cal_i8x4_unpack((uint32_t)cal_i8x8_pack(x));
}

#ifdef __cplusplus
}
#endif
#include "compat/inline_end.h"
#include "compat/alignas_end.h"
#endif
