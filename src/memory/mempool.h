#ifndef _TH_MEMPOOL_H
#define _TH_MEMPOOL_H

#include "list.h"


#define SLAB_CONSISTENCY_CHECKS	0x00000100UL	/* DEBUG: Perform (expensive) checks on alloc/free */
#define SLAB_RED_ZONE		0x00000400UL	/* DEBUG: Red zone objs in a cache */
#define SLAB_POISON		0x00000800UL	/* DEBUG: Poison objects */
#define SLAB_HWCACHE_ALIGN	0x00002000UL	/* Align objs on cache lines */
#define SLAB_CACHE_DMA		0x00004000UL	/* Use GFP_DMA memory */
#define SLAB_STORE_USER		0x00010000UL	/* DEBUG: Store the last owner for bug hunting */
#define SLAB_PANIC		0x00040000UL	/* Panic if kmem_cache_create() fails */
#define SLAB_CORE_FLAGS SLAB_HWCACHE_ALIGN 

#define CACHE_CREATE_MASK (SLAB_CORE_FLAGS)

struct mempool_cache_node {
	struct list_head slabs_partial;	/* partial list first, better asm code */
	struct list_head slabs_full;
	struct list_head slabs_free;
	unsigned long num_slabs;
	unsigned long free_objects;
	unsigned int free_limit;
	unsigned int colour_next;	/* Per-node cache coloring */
	unsigned long next_reap;	/* updated without locking */
	int free_touched;		/* updated without locking */
};

struct mempool {
	unsigned int size;
/* 2) touched by every alloc & free from the backend */
	unsigned int flags;		/* constant flags */
	unsigned int num;		/* # of objs per slab */

/* 3) cache_grow/shrink */
	/* order of pgs per slab (2^n) */
	unsigned int order;

	size_t colour;			/* cache colouring range */
	unsigned int colour_off;	/* colour offset */

	struct mempool *freelist_cache;
	unsigned int freelist_size;

	/* constructor func */
	void (*ctor)(void *obj);

/* 4) cache creation/removal */
	const char *name;
	struct list_head list;
	int refcount;
	int object_size;
	int align;
	struct mempool_cache_node node;
};

#endif
