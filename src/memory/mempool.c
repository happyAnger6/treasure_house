#include "types.h"
#include "list.h"
#include "error.h"

#include "mempool.h"

#define PAGE_SIZE 4096
#define MALLOC_MAX_ORDER 6
LIST_HEAD(mempool_list);

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

static mempool* find_mempool_by_name(const char *name)
{
    struct mempool* pool_p;
    list_for_each_entry(pool_p, mempool_list, pool_list){
        if(!strcmp(pool_p->name, name)){
            return pool_p;
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

        /* Can't handle number of objects more than SLAB_OBJ_MAX_NUM 
        if (num > SLAB_OBJ_MAX_NUM)
            break; */

        /* Found something acceptable - save it away */
        mempoolp->num = num;
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

static bool set_off_slab_mempool(struct mempool *poolp, size_t size, unsigned long flags)
{
    poolp->num = 0;

    calculate_slab_order(poolp, size, flags);
    if(!poolp->num){
        return false;
    }

    return true;
}

static void init_slabs_info(struct mempool *poolp)
{
    INIT_LIST_HEAD(&(poolp->slabs_info.slab_full));
    INIT_LIST_HEAD(&(poolp->slabs_info.slab_partial));
    INIT_LIST_HEAD(&(poolp->slabs_info.slab_free));
    poolp->slabs_info.cur_slab = NULL;
    poolp->slabs_info.slab_nums = 0;
    poolp->slabs_info.free_objects = 0;
}

static int 
struct mempool* mempool_create(const char *name, size_t obj_size,
				size_t align, mempool_ops *ops_p, 
                void *private, unsigned long flags)
{
    if(NULL == name)
    {
        return NULL;
    }

    struct mempool* pool_p;
    pool_p = find_mempool_by_name(name);
    if(NULL != pool_p)
    {
        return NULL;
    }

    pool_p = (struct mempool *)calloc(sizeof(struct mempool));
    if(NULL == pool_p)
    {
        return NULL;
    }

    pool_p->name = strdup(name);
    pool_p->object_size = obj_size;
    pool_p->align = calculate_alignment(flags, align, obj_size);
    pool_p->size = ALIGN(obj_size, pool_p->align):
    pool_p->ops = ops_p;

    if (!set_off_slab_mempool(pool_p, pool_p->size, flags)) {
        free(pool_p);
        return ERROR_FAILED;
    }

	s->refcount = 1;
	list_add(&s->pool_list, &mempool_list);
    return ERROR_SUCCESS;
}

static __always_inline void *
slab_alloc(struct mempool *mempoolp, gfp_t flags)
{
    unsigned int order = mempoolp->order;
    size_t slab_size = PAGE_SIZE << order;
    struct slab *slabp = calloc(sizeof(struct slab));
    if(!slabp)
    {
        return ERROR_FAILED;
    }

    void *basep = malloc(slab_size);
    if(!basep)
    {
        free(slabp);
        return ERROR_FAILED;
    }

    void *obj_basep = ALIGN_ADDR(basep, mempoolp->align);
    slabp->s_base = slabp;
    slabp->s_base = slabp;
}

static struct slab *load_slab(struct mempool *mempoolp)
{
    struct slab *slab = NULL;
    struct mempool_slabs_info *slabs = &mempoolp->slabs;
    list_for_each_entry(slab, &slabs->partial_list, list) {
        if(slab->free > 0){
            mempoolp->
        }
    }

}

static void *find_slab(struct mempool *mempoolp)
{
    struct slab *slab = mempoolp->slabs->cur_slab;
    struct slabs_info *slabs = &mempoolp->slabs;

    if(slab->free == 0)
    {
        MOVE_SLAB_LIST(slab, &slabs->full_list);
        slab = NULL;
        slab = load_slab(mempoolp);
    }

    return slab;
}

/**
 * mempool_alloc - Allocate an object
 * @mempoolp: The cache to allocate from.
 * @flags: See kmalloc().
 *
 * Allocate an object from this cache.  The flags are only relevant
 * if the cache has no available objects.
 */
void *mempool_alloc(struct mempool *mempoolp, gfp_t flags)
{
    struct slabs_info *slabs = &mempoolp->slabs;
    struct slab *slab = NULL;
    void *ret = NULL;

    if(slabs->free_objects == 0){
        ret = slab_alloc(mempoolp, flags);
    }

    slab = find_slab(mempoolp);

}