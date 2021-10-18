#pragma once

#include <stdint.h>

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef   int8_t  s8;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;
typedef    float f32;
typedef   double f64;

#if __has_builtin(__builtin_offsetof)
#define OffsetOf(x, y) __builtin_offsetof(x, y)
#else
#define OffsetOf(x, y) (u64)(&((x *)nullptr)->y)
#endif

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

#include <assert.h>

