#ifndef _COROUTINE_BITOPS_H
#define _COROUTINE_BITOPS_H

#define BITS_PER_BYTE 8
#define DIV_ROUND_UP(n, d) ((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr) (DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#endif