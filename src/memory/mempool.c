#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "list.h"
#include "error.h"

#include "mempool.h"

#define PAGE_SIZE 4096
#define MALLOC_MAX_ORDER 6

static LIST_HEAD(mempool_list);

#define MOVE_SLAB_LIST(old, new) \
    do {                        \
        list_del(&(old)->list); \
        list_add(&(old)->list, (new)); \
    }while (0)
/*
 * Figure out what the alignment of the objects will be given a set of
 * flags, a user specified alignment and the size of the objects.
 */
static unsigned long calculate_alignment(unsigned long flags,
		unsigned long align, unsigned long size)
{
	/*
	 * If the user wants hardware cache aligned objects then follow that
	 * suggestion if the object is sufficiently large.
	 *
	 * The hardware cache alignment cannot override the specified
	 * alignment though. If that is greater then use it.
	 *
	if (flags & SLAB_HWCACHE_ALIGN) {
		unsigned long ralign = cache_line_size();
		while (size <= ralign / 2)
			ralign /= 2;
		align = max(align, ralign);
	}*/

	return ALIGN(align, sizeof(void *));
}

static struct mempool* find_mempool_by_name(const char *name)
{
    struct mempool* poolp;
    list_for_each_entry(poolp, &mempool_list, pool_list){
        if(!strcmp(poolp->name, name)){
            return poolp;
        }
    }

    return NULL;
}

/*
 * Calculate the number of objects and left-over bytes for a given buffer size.
 */
static unsigned int mempool_estimate(unsigned long order, size_t buffer_size,
        size_t align, unsigned long flags, size_t *left_over)
{
    unsigned int num;
    size_t slab_size = PAGE_SIZE << order;
    size_t sub_align_size =(slab_size - align);

    /*
     * The slab management structure can be either off the slab or
     * on it. For the latter case, the memory allocated for a
     * slab is used for:
     *
     * - @buffer_size bytes for each object
     * - One freelist_idx_t for each object
     *
     * We don't need to consider alignment of freelist because
     * freelist will be at the end of slab page. The objects will be
     * at the correct alignment.
     *
     * If the slab management structure is off the slab, then the
     * alignment will already be calculated into the size. Because
     * the slabs are all pages aligned, the objects will be at the
     * correct alignment when allocated.
     */
    num = sub_align_size / buffer_size;
    *left_over = sub_align_size % buffer_size; 

    return num;
}

/**
 * calculate_slab_order - calculate size (page order) of slabs
 * @mempoolp: pointer to the cache that is being created
 * @size: size of objects to be created in this cache.
 * @flags: slab allocation flags
 *
 * Also calculates the number of objects per slab.
 *
 * This could be made much more intelligent.  For now, try to avoid using
 * high order pages for slabs.  When the gfp() functions are more friendly
 * towards high-order requests, this should be changed.
 */
static size_t calculate_slab_order(struct mempool *mempoolp,
                size_t size, unsigned long flags)
{
    size_t left_over = 0;
    int order;

    for (order = 0; order <= MALLOC_MAX_ORDER; order++) {
        unsigned int num;
        size_t remainder;

        num = mempool_estimate(order, size, mempoolp->align, flags, &remainder);
        if (!num)
            continue;

        /* Can't handle number of objects more than SLAB_OBJ_MAX_NUM */
        if (num < SLAB_OBJ_MIN_NUM)
            continue; 

        /* Found something acceptable - save it away */
        mempoolp->num_per_slab = num;
        mempoolp->order = order;
        left_over = remainder;

        /*
         * Acceptable internal fragmentation?
         */
        if (left_over * 8 <= (PAGE_SIZE << order))
            break;
    }
    return left_over;
}

static bool set_off_slab_mempool(struct mempool *pool, size_t size, unsigned long flags)
{
    pool->num_per_slab = 0;

    calculate_slab_order(pool, size, flags);
    if (!pool->num_per_slab) {
        return false;
    }

    return true;
}

static void init_slabs_info(struct slabs_info *slabs)
{
    INIT_LIST_HEAD(&slabs->full);
    INIT_LIST_HEAD(&slabs->partial);
    INIT_LIST_HEAD(&slabs->empty);
    slabs->cur = NULL;
    slabs->nums = 0;
    slabs->free_objs = 0;
}

struct mempool* mempool_create(const char *name, size_t obj_size,
				size_t align, struct mempool_ops *ops_p, 
                void *private, unsigned long flags)
{
    struct mempool* pool;
    
    if (NULL == name)
    {
        return NULL;
    }

    if (NULL != find_mempool_by_name(name))
    {
        return NULL;
    }

    pool = (struct mempool *)calloc(1, sizeof(struct mempool));
    if (NULL == pool)
    {
        return NULL;
    }

    pool->name = strdup(name);
    pool->obj_size = obj_size;
    pool->align = calculate_alignment(flags, align, obj_size);
    pool->size = ALIGN(obj_size, pool->align);
    pool->ops = ops_p;

    if (!set_off_slab_mempool(pool, pool->size, flags)) {
        free(pool);
        return NULL;
    }

	pool->refcount = 1;
	list_add(&pool->pool_list, &mempool_list);
    init_slabs_info(&pool->slabs);
    return pool;
}

static void init_freelist(freelist_idx_t *free_list, size_t num)
{
    for(size_t i = 0; i < num - 1; i++)
    {
        free_list[i] = i + 1;
    }
    free_list[num-1] = -1;
}

static __always_inline struct slab *
slab_alloc(struct mempool *mempoolp, unsigned long flags)
{
    size_t num = mempoolp->num_per_slab;
    size_t slab_size = PAGE_SIZE << mempoolp->order;
    struct slabs_info *slabs = &mempoolp->slabs;
    struct slab *slab = calloc(1, slab_size + num * sizeof(freelist_idx_t));
    if (!slab)
    {
        return NULL;
    }

    void *base = malloc(slab_size);
    if (!base)
    {
        goto out_free_slab;
    }

    init_freelist(slab->freelist, num);
    void *obj_base = ALIGN_ADDR(base, mempoolp->align);
    slab->s_base = base;
    slab->obj_base = obj_base;
    slab->free = num;
    slab->used = 0;
    slab->free_idx = 0;

    list_add(&slab->list, &slabs->empty);
    slabs->nums += 1;
    slabs->free_objs += num;
    slabs->cur = slab;
    return slab;

out_free_slab:
    free(slab);
    return NULL;
}

static struct slab *load_slab(struct mempool *mempoolp)
{
    struct slab *slab = NULL;
    struct slabs_info *slabs = &mempoolp->slabs;
    list_for_each_entry(slab, &slabs->partial, list) {
        if(slab->free > 0){
            return slab;
        }
    }

    return NULL;
}

static void *find_slab(struct slabs_info *slabs)
{
    struct slab *slab = slabs->cur;

    if (slab->free == 0)
    {
        slab = load_slab(slabs);
    }

    return slab;
}

static void *alloc_obj_from_slab(struct slab *slab, struct mempool *pool)
{
    short free_idx = slab->free_idx;
    assert(free_idx != -1);

    slab->free_idx = slab->freelist[slab->free_idx];
    slab->free -= 1;
    slab->used += 1;
    if (slab->free == 0) {
        MOVE_SLAB_LIST(slab, &((pool->slabs).full));
    }

    return slab->obj_base + free_idx * pool->size;
}

/**
 * mempool_alloc - Allocate an object
 * @mempoolp: The cache to allocate from.
 * @flags: See kmalloc().
 *
 * Allocate an object from this cache.  The flags are only relevant
 * if the cache has no available objects.
 */
void *mempool_alloc(struct mempool *mempoolp, unsigned long flags)
{
    struct slabs_info *slabs = &mempoolp->slabs;
    struct slab *slab = NULL;
    void *obj = NULL;

    if (slabs->free_objs == 0)
    {
        slab = slab_alloc(mempoolp, flags);
        if (!slab) {
            return NULL;
        }
        goto find_slab;
    }

    slab = find_slab(slabs);
    if (!slab) {
        return NULL;
    }

find_slab:
    obj = alloc_obj_from_slab(slab, mempoolp);
    if (obj) {
        slabs->free_objs -= 1;
    }
    return obj;
}

void mempool_destory(struct mempool* mempool_p)
{
    return;
}