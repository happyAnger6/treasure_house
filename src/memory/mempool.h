#ifndef _TH_MEMPOOL_H
#define _TH_MEMPOOL_H

#include "treasure_house/spinlock.h"

#define MEMPOOL_CONSIST_CHECKS 0x00000100UL
#define MEMPOOL_RED_ZONE 0x00000400UL
#define MEMPOOL_POSION 0x00000800UL
#define MEMPOOL_HWCACHE_ALIGN 0x00002000UL

typedef struct th_mempool_s{
	struct list_head        mem_block_full;
   	struct list_head        mem_block_partial;
   	struct list_head        mem_block_free;
    unsigned int            objsize;
    unsigned int            flags;
    unsigned int            num;
	spinlock_t 				spinlock;	
	unsigned int 			order;
	unsigned int 			flags;
	size_t 					color;
	unsigned int 			color_off;
	unsigned int 			color_next;
	struct th_mempool_s     *mempool_next_p;
	unsigned int 			growing;
	unsigned int 			dflags;
	void (*ctor)(void *, th_mempool_t *, unsigned long);
	void (*dtor)(void *, th_mempool_t *, unsigned long);
	unsigned long           failures;
}th_mempool_t;

typedef void* th_mempool_handle;
typedef unsigned int mem_bufctl_t;

typedef struct th_memblock_s {
	struct list_head list;
	unsigned long color_off;
	void *object_mem;
	unsigned int inuse;
	mem_bufctl_t free[];
}th_memblock_t;

#define memblock_bufctl(mblock_p)	\
		((mem_bufctl_t *)(((th_memblock_t *)mblock_p) + 1))

/* Create a new mempool */
th_mempool_t* th_mempool_create(const char *name, size_t size,
    size_t align, unsigned long flags,
    void (*ctor)(void*, th_mempool_t*, unsigned long),
    void (*dtor)(void*, th_mempool_t*, unsigned long));


/* Delete all mempool_block in the free list*/
th_mempool_shrink(th_mempool_t *mempool_p);

/* Allocate a single object from the mempool and return it to the caller */
void* th_mempool_alloc(th_mempool_t *mempool_p);

/* Free an object from the mempool and return it to the mempool */
void th_mempool_free(th_mempool_t *mempool_p, void *obj_p);

/* Destory all objects in all mempool_block and frees up all associated memory */
int th_mempool_destory(th_mempool_t *mempool_p);

int th_mempool_reap(int mask);

    
#endif
