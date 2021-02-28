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
	short free_idx;
	unsigned short used;
	unsigned short free;
    freelist_idx_t freelist[0];     /* sl[aou]b first free object */
};

struct slabs_info{
	struct list_head partial;	/* partial list first, better asm code */
	struct list_head full;
	struct list_head empty;
	struct slab *cur;
	unsigned long nums;
	unsigned long free_objs;
};

struct mempool_ops {
	int (*ctor)(void *obj, void *priv, int flags);
	int (*dctor)(void *obj, void *priv);
	void (*reclaim)(void *priv);
};

struct mempool {
	struct list_head pool_list;
	const char *name;
	unsigned int size;
	int obj_size;
	int align;
	unsigned int flags;		/* constant flags */
	unsigned int num_per_slab;		/* # of objs per slab */
	unsigned int order;
	int refcount;
	struct mempool_ops *ops;
	struct slabs_info slabs;
};

extern struct mempool *mempool_create(const char *name, size_t obj_size,
				size_t align, struct mempool_ops *opsp,
				void *priv, unsigned long flags);

extern void* mempool_alloc(struct mempool* mempool_p, unsigned long flags);
extern void* mempool_free(struct mempool* mempool_p, void *obj);
extern void mempool_destory(struct mempool* mempool_p);

#ifdef __cplusplus
}
#endif

#endif
