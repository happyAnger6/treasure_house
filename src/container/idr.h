#ifndef _COROUTINE_IDR_H
#define _COROUTINE_IDR_H 

#ifdef __cplusplus
extern "C"{
#endif

#include "types.h"
#include "co_spinlock.h"

#define IDR_BITS 8
#define IDR_SIZE (1<<IDR_BITS)

struct idr_layer {
    int prefix; /* the ID prefix of this layer */
    int layer; /* distance from leaf*/
    struct idr_layer *ary[1<<IDR_BITS];
    int count; /* when zero, we can release it. */
    DECLARE_BITMAP(bitmap, IDR_SIZE);
}

struct idr {
    struct idr_layer *hint;
    struct idr_layer *top;
    int layers;
    int cur;
    co_spin_lock_t lock;
    int id_free_cnt;
    struct idr_layer *id_free;
}

#ifdef __cplusplus
}
#endif

#endif