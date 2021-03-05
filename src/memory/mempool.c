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

#define MOVE_LIST(old, new) \
    do {                        \
        list_del(&(old)->list); \
        list_add(&(old)->list, (new)); \
    }while (0)

#define CHARP_OFFSET(p, size) \
    (typeof(p))((char *)(p) + (size))

#define ADDR_IN_SLAB(slab, addr) \
    ((slab)->obj_base <= addr && addr <= (slab)->obj_end)

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
    struct mempool* pool;
    list_for_each_entry(pool, &mempool_list, list){
        if(!strcmp(pool->name, name)){
            return pool;
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
    slabs->slab_nums = 0;
    slabs->total_objs = 0;
    slabs->used_objs = 0;
    slabs->free_objs = 0;
}

struct mempool* mempool_create(const char *name, size_t obj_size,
                size_t align, struct mempool_ops *ops, 
                void *private, unsigned long flags)
{
    struct mempool* pool;
    
    if (NULL == name)
        return NULL;

    if (NULL != find_mempool_by_name(name))
        return NULL;

    pool = (struct mempool *)calloc(1, sizeof(struct mempool));
    if (NULL == pool)
        return NULL;

    pool->name = strdup(name);
    pool->obj_size = obj_size;
    pool->align = calculate_alignment(flags, align, obj_size);
    pool->size = ALIGN(obj_size, pool->align);
    pool->ops = ops;

    if (!set_off_slab_mempool(pool, pool->size, flags)) {
        free(pool->name);
        free(pool);
        return NULL;
    }

    pool->refcount = 1;
    pool->priv = private;
    list_add(&pool->list, &mempool_list);
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

static struct slab *slab_alloc(struct mempool *mempool, unsigned long flags)
{
    size_t num_per_slab = mempool->num_per_slab;
    size_t slab_size = PAGE_SIZE << mempool->order;
    struct slabs_info *slabs = &mempool->slabs;
    struct slab *slab = calloc(1, slab_size + num_per_slab * sizeof(freelist_idx_t));
    if (!slab)
    {
        return NULL;
    }

    void *base = malloc(slab_size);
    if (!base)
    {
        goto out_free_slab;
    }

    slab->free_idx = 0;
    slab->last_free_idx = num_per_slab - 1;
    init_freelist(slab->freelist, num_per_slab);
    
    slab->s_base = base;
    slab->obj_base = ALIGN_ADDR(base, mempool->align);
    slab->obj_end = CHARP_OFFSET(slab->obj_base, num_per_slab * mempool->size);
    slab->free = num_per_slab;
    slab->used = 0;

    list_add(&slab->list, &slabs->empty);
    slabs->slab_nums += 1;
    slabs->free_objs += num_per_slab;
    slabs->total_objs += num_per_slab;
    slabs->cur = slab;
    return slab;

out_free_slab:
    free(slab);
    return NULL;
}

static struct slab *load_slab(struct slabs_info *slabs)
{
    struct slab *slab = NULL;
    list_for_each_entry(slab, &slabs->partial, list) {
        if(slab->free > 0) {
            slabs->cur = slab;
            return slab;
        }
    }

    list_for_each_entry(slab, &slabs->empty, list) {
        if(slab->free > 0) {
            slabs->cur = slab;
            return slab;
        }
    }

    return NULL;
}

static void *find_slab(struct slabs_info *slabs)
{
    struct slab *slab = slabs->cur;

    if (slab == NULL || slab->free == 0)
    {
        slab = load_slab(slabs);
    }

    return slab;
}

static void *alloc_obj_from_slab(struct slab *slab, struct mempool *pool)
{
    struct slabs_info *slabs = &pool->slabs;

    short free_idx = slab->free_idx;
    assert(free_idx != -1);

    /*header first*/
    slab->free_idx = slab->freelist[slab->free_idx];
    slab->freelist[free_idx] = -1;
    slab->free -= 1;
    slab->used += 1;

    /*last obj in slab*/
    if (free_idx == slab->last_free_idx) {
        slab->last_free_idx == -1;
        MOVE_LIST(slab, &pool->slabs.full);
    }

    if (slab->used == 1)
        MOVE_LIST(slab, &pool->slabs.partial);

    slabs->free_objs -= 1;
    slabs->used_objs += 1;
    return CHARP_OFFSET(slab->obj_base, free_idx * pool->size);
}

/**
 * mempool_alloc - Allocate an object
 * @mempoolp: The mempool to allocate from.
 * @flags: reserved.
 *
 * Allocate an object from this mempool.  The flags are only relevant
 * if the mempool has no available objects.
 */
void *mempool_alloc(struct mempool *mempoolp, unsigned long flags)
{
    struct slabs_info *slabs = &mempoolp->slabs;
    struct slab *slab = NULL;
    void *obj = NULL;
    
    if (slabs->free_objs == 0) {
        slab = slab_alloc(mempoolp, flags);
        if (!slab)
            return NULL;
            
        goto find_a_slab;
    }

    slab = find_slab(slabs);
    if (!slab)
        return NULL;

find_a_slab:
    obj = alloc_obj_from_slab(slab, mempoolp);
    if(obj && mempoolp->ops && mempoolp->ops->ctor)
        mempoolp->ops->ctor(obj, mempoolp->priv, flags);
    
    return obj;
}

static struct slab *find_obj_slab(struct mempool *pool, void *obj)
{
    struct slabs_info *slabs = &pool->slabs;
    struct slab *slab = NULL;
    
    if (slabs->used_objs == 0)
        return NULL;

    list_for_each_entry(slab, &slabs->partial, list) {
        if (slab->used > 0 && ADDR_IN_SLAB(slab, obj)){
            return slab;
        }
    }

    list_for_each_entry(slab, &slabs->full, list) {
        if (slab->used > 0 && ADDR_IN_SLAB(slab, obj)){
            return slab;
        }
    }

    return NULL;
}

static void free_all_objs_one_slab(struct mempool *pool, struct slab *slab)
{
    struct slabs_info *slabs = &pool->slabs;
    struct mempool_ops *ops = pool->ops;

    freelist_idx_t *freelist = slab->freelist;
    freelist_idx_t free_idx = slab->free_idx;
    freelist_idx_t last_free_idx = slab->last_free_idx;
    void *obj = NULL;
    
    for (size_t i = 0; i < pool->num_per_slab; i++)
    {
        obj = CHARP_OFFSET(slab->obj_base, i * pool->size);
        
        /*free_idx == -1 means all objs are in used;
              * freelist[i] == -1 && i != last_free_idx means i is in used.
            */
        if (free_idx == -1 || (freelist[i] == -1 && i != last_free_idx))
        {
            if(ops && ops->dctor)
                ops->dctor(obj, pool->priv);
            
            slabs->used_objs -= 1;
        } 
        else 
        {
            slabs->free_objs -= 1;
        }

        slabs->total_objs -= 1;
    }
}

static void free_one_slab(struct mempool *pool, struct slab *slab)
{
    struct slabs_info *slabs = &pool->slabs;

    free_all_objs_one_slab(pool, slab);
    list_del(&slab->list);
    free(slab->s_base);
    free(slab);

    slabs->slab_nums -= 1;
}

void free_obj2slab(void *obj, struct slab *slab, struct mempool *pool)
{
    struct slabs_info *slabs = &pool->slabs;
    short free_idx = (size_t)(obj - slab->obj_base) / pool->size;
    
    freelist_idx_t *free_list = slab->freelist;
    free_list[free_idx] = slab->free_idx;
    slab->free_idx = free_idx;
    
    slab->free += 1;
    slab->used -= 1;
    
    if (slab->used == 0) {
        if (!list_empty(&slabs->empty))
            free_one_slab(pool, slab); /*keep only one empty slab*/
        else
            MOVE_LIST(slab, &slabs->empty);
    }

    /*first free obj in slab*/
    if (slab->last_free_idx == -1) {
        slab->last_free_idx = free_idx;
        MOVE_LIST(slab, &slabs->partial);
    }

    slabs->free_objs += 1;
    slabs->used_objs -= 1;
}

void mempool_free(struct mempool* mempool_p, void *obj)
{
    struct slab *slab = NULL;
    
    if (NULL == mempool_p)
        return;

    slab = find_obj_slab(mempool_p, obj);
    if (slab)
        free_obj2slab(obj, slab, mempool_p);

    return;
}

static void free_all_slabs(struct mempool *pool)
{
    struct slabs_info *slabs = &pool->slabs;
    struct slab *slab = NULL, *tmp_slab = NULL;
    
    if (slabs->slab_nums == 0)
        return;

    list_for_each_entry_safe(slab, tmp_slab, &slabs->partial, list) {
        free_one_slab(pool, slab);
    }

    list_for_each_entry_safe(slab, tmp_slab, &slabs->full, list) {
        free_one_slab(pool, slab);
    }

    list_for_each_entry_safe(slab, tmp_slab, &slabs->empty, list) {
        free_one_slab(pool, slab);
    }
}

void mempool_destory(struct mempool* mempool_p)
{
    if (mempool_p == NULL)
        return;
    
    free_all_slabs(mempool_p);

    list_del(&mempool_p->list);
    free(mempool_p->name);
    free(mempool_p);
    return;
}