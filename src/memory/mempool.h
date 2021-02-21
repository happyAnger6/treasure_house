#ifndef _TH_MEMPOOL_H
#define _TH_MEMPOOL_H

#include "list.h"
#define SLAB_OBJ_MIN_SIZE 32

typedef unsigned short freelist_idx_t

struct slab {
	struct list_head list;
	void *s_base;
    void *obj_base;            /* slab first object */
	short free_idx;
	unsigned short total;
	unsigned short used;
	unsigned short free;
    freelist_idx_t freelist[0];     /* sl[aou]b first free object */
};

struct slabs_info{
	struct list_head partial_list;	/* partial list first, better asm code */
	struct list_head full_list;
	struct list_head free_list;
	struct slab *cur_slab;
	unsigned long slab_nums;
	unsigned long free_objects;
};

struct mempool_ops {
	int (*ctor)(void *obj, void *private, int flags);
	int (*dctor)(void *obj, void *private);
	void (*reclaim)(void *private);
};

struct mempool {
	struct list_head pool_list;
	const char *name;
	unsigned int size;
	int object_size;
	int align;
	unsigned int flags;		/* constant flags */
	unsigned int num;		/* # of objs per slab */
	unsigned int order;
	int refcount;
	struct mempool_ops *ops;
	struct slabs_info slabs;
};

extern struct mempool* mempool_create(const char *name, size_t obj_size,
				size_t align,
				mempool_ops *ops_p,
				void *private, int flags);

extern void* mempool_alloc(struct mempool* mempool_p, int flags);
extern void* mempool_free(struct mempool* mempool_p, void *obj);
extern void mempool_destory(struct mempool* mempool_p);
#endif
