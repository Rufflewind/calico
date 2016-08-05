#ifndef G_ED64P8RGJN8XDUAT6RENZ7I7LS2EP
#define G_ED64P8RGJN8XDUAT6RENZ7I7LS2EP
#include <stdint.h>
#ifdef __SSE2__
# include <emmintrin.h>
#endif
#include "arithmetic.h"
#define CAL_ALIGNAS_OPTIONAL
#include "compat/alignas_begin.h"
#include "compat/inline_begin.h"
#ifdef __cplusplus
extern "C" {
#endif
/*@self.public()*/

struct cal_i32x2 {
    alignas(8) int32_t array[2];
};

static inline
uint64_t cal_pack_i32x2(struct cal_i32x2 x)
{
    return
        (uint64_t)(uint32_t)x.array[0] << 0 |
        (uint64_t)(uint32_t)x.array[1] << 32;
}

static inline
struct cal_i32x2 cal_unpack_i32x2(uint64_t x)
{
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

/** Add 2 packed 32-bit integers. */
static inline
struct cal_i32x2 cal_add_i32x2(struct cal_i32x2 x, struct cal_i32x2 y)
{
#ifdef __SSE2__
    return cal_unpack_i32x2(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_add_epi32(_mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i32x2(x))),
                          _mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i32x2(y))))));
#else
    size_t i;
    struct cal_i32x2 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] + y.array[i];
    }
    return z;
#endif
}

/** Subtract 2 packed 32-bit integers. */
static inline
struct cal_i32x2 cal_sub_i32x2(struct cal_i32x2 x, struct cal_i32x2 y)
{
#ifdef __SSE2__
    return cal_unpack_i32x2(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_sub_epi32(_mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i32x2(x))),
                          _mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i32x2(y))))));
#else
    size_t i;
    struct cal_i32x2 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] - y.array[i];
    }
    return z;
#endif
}

struct cal_i8x8 {
    alignas(8) int8_t array[8];
};

static inline
uint64_t cal_pack_i8x8(struct cal_i8x8 x)
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

static inline
struct cal_i8x8 cal_unpack_i8x8(uint64_t x)
{
#if defined __GNUC__ &&                                                     \
    defined __BYTE_ORDER__ &&                                               \
    defined __ORDER_LITTLE_ENDIAN__ &&                                      \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
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

/** Add 8 packed 8-bit integers. */
static inline
struct cal_i8x8 cal_add_i8x8(struct cal_i8x8 x, struct cal_i8x8 y)
{
    /* unlike ICC 13.0, Clang 3.7 and GCC 5.3 weren't able to optimize the
       naive loop into a simple instruction */
#ifdef __SSE2__
    return cal_unpack_i8x8(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_add_epi8(_mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i8x8(x))),
                         _mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i8x8(y))))));
#else
    size_t i;
    struct cal_i8x8 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] + y.array[i];
    }
    return z;
#endif
}

/** Subtract 8 packed 8-bit integers. */
static inline
struct cal_i8x8 cal_sub_i8x8(struct cal_i8x8 x, struct cal_i8x8 y)
{
#ifdef __SSE2__
    return cal_unpack_i8x8(
        (uint64_t)_mm_cvtsi128_si64(
            _mm_sub_epi8(_mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i8x8(x))),
                         _mm_cvtsi64_si128(cal_u64_to_i64(cal_pack_i8x8(y))))));
#else
    size_t i;
    struct cal_i8x8 z;
    for (i = 0; i != sizeof(z.array) / sizeof(*z.array); ++i) {
        z.array[i] = x.array[i] - y.array[i];
    }
    return z;
#endif
}

struct cal_i8x4 {
    alignas(4) int8_t array[4];
};

static inline
struct cal_i8x4 cal_unpack_i8x4(uint32_t i)
{
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

static inline
uint32_t cal_pack_i8x4(struct cal_i8x4 t)
{
    /* this should optimize to a simple mov */
    return
        (uint32_t)(uint8_t)t.array[0] << 0 |
        (uint32_t)(uint8_t)t.array[1] << 8 |
        (uint32_t)(uint8_t)t.array[2] << 16 |
        (uint32_t)(uint8_t)t.array[3] << 24;
}

static inline
struct cal_i8x8 cal_combine_i8x4_to_i8x8(struct cal_i8x4 x0, struct cal_i8x4 x1)
{
    return cal_unpack_i8x8(
        (uint64_t)cal_pack_i8x4(x0) << 0 |
        (uint64_t)cal_pack_i8x4(x1) << 32
    );
}

static inline
struct cal_i8x4 cal_lower_i8x4_of_i8x8(struct cal_i8x8 x)
{
    return cal_unpack_i8x4((uint32_t)cal_pack_i8x8(x));
}

static inline
struct cal_i8x8 cal_i8x4_to_i8x8(struct cal_i8x4 x)
{
    struct cal_i8x4 z = {{0}};
    return cal_combine_i8x4_to_i8x8(x, z);
}

static inline
struct cal_i8x4 cal_i8x8_to_i8x4(struct cal_i8x8 x)
{
    return cal_unpack_i8x4((uint32_t)cal_pack_i8x8(x));
}

static inline
struct cal_i8x4 cal_add_i8x4(struct cal_i8x4 x, struct cal_i8x4 y)
{
    /* adding them as packed integers is twice as fast as the naive approach */
    return cal_i8x8_to_i8x4(cal_add_i8x8(cal_i8x4_to_i8x8(x), cal_i8x4_to_i8x8(y)));
}

static inline
struct cal_i8x4 cal_sub_i8x4(struct cal_i8x4 x, struct cal_i8x4 y)
{
    return cal_i8x8_to_i8x4(cal_sub_i8x8(cal_i8x4_to_i8x8(x), cal_i8x4_to_i8x8(y)));
}

#ifdef __cplusplus
}
#endif
#include "compat/inline_end.h"
#include "compat/alignas_end.h"
#undef CAL_ALIGNAS_OPTIONAL
#endif
