#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PI    (3.1415926535897932384626433832795f)
#define PI180 (3.1415926535897932384626433832795f / 180.f)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define countof(x) (sizeof(x)/sizeof(*x))

typedef unsigned int     uint;
typedef unsigned short ushort;
typedef unsigned char   uchar;

typedef uint64_t       u64;
typedef uint32_t       u32;
typedef uint16_t       u16;
typedef uint8_t         u8;

typedef  int64_t       i64;
typedef  int32_t       i32;
typedef  int16_t       i16;
typedef  int8_t         i8;
