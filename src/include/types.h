#ifndef _TH_TYPES_H
#define _TH_TYPES_H

#ifdef __cplusplus
extern "C"{
#endif

#include "bitops.h"

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef unsigned gfp_t;

#define BYTES_PER_WORD      sizeof(void *)
#define BITS_PER_BYTE 8

#define __ALIGN_MASK(x, mask)   (((x) + (mask)) & ~(mask))
#define __ALIGN(x, a)       __ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN(x, a) __ALIGN((x), (a))
#define ALIGN_ADDR(addr, align) \
    (typeof(addr))(ALIGN(((intptr_t)(addr)), align))

#define max(x, y)      \
    (x) > (y) ? (x) : (y)

#define MIN(a, b) ((a)<(b) ? (a):(b))

#define  noinline   __attribute__((noinline))

#define DECLARE_BITMAP(name, bits) \
    unsigned long name[BITS_TO_LONGS(bits)]

#ifdef __cplusplus
}
#endif

#endif