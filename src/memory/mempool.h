#ifndef _TH_MEMPOOL_H
#define _TH_MEMPOOL_H

#ifdef __cplusplus
extern "C"{
#endif

#include "list.h"
#define SLAB_OBJ_MIN_SIZE 32
#define SLAB_OBJ_MIN_NUM 8

typedef unsigned short freelist_idx_t;

struct slab {
    struct list_head list;
    void *s_base;
    void *obj_base;            /* slab first object */
    void *obj_end;
    short free_idx;
    short last_free_idx;
    unsigned short used;
    unsigned short free;
    freelist_idx_t freelist[0];     /* sl[aou]b first free object */
};

struct slabs_info{
    struct list_head partial;   /* partial list first, better asm code */
    struct list_head full;
    struct list_head empty;
    struct slab *cur;
    unsigned long slab_nums;
    unsigned long total_objs;
    unsigned long used_objs;
    unsigned long free_objs;
};

struct mempool_ops {
    int (*ctor)(void *obj, void *priv, int flags);
    int (*dctor)(void *obj, void *priv);
    void (*reclaim)(void *priv);
};

struct mempool {
    struct list_head list;
    char *name;
    unsigned int size;
    int obj_size;
    int align;
    unsigned int flags;     /* constant flags */
    unsigned int num_per_slab;      /* objs per slab */
    unsigned int order;
    int refcount;
    void *priv;
    struct mempool_ops *ops;
    struct slabs_info slabs;
};

extern struct mempool *mempool_create(const char *name, size_t obj_size,
                size_t align, struct mempool_ops *opsp,
                void *priv, unsigned long flags);

extern void* mempool_alloc(struct mempool* mempool_p, unsigned long flags);
extern void mempool_free(struct mempool* mempool_p, void *obj);
extern void mempool_destory(struct mempool* mempool_p);

#ifdef __cplusplus
}
#endif

#endif
