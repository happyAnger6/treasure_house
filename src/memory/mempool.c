#include "types.h"
#include "list.h"
#include "error.h"

#include "mempool.h"

#define CFLGS_OBJFREELIST_SLAB  (0x40000000UL)
#define CFLGS_OFF_SLAB      (0x80000000UL)
#define OBJFREELIST_SLAB(x) ((x)->flags & CFLGS_OBJFREELIST_SLAB)
#define OFF_SLAB(x) ((x)->flags & CFLGS_OFF_SLAB)

#define PAGE_SIZE 4096
#define SLAB_OBJ_MIN_SIZE 16
#define FREELIST_BYTE_INDEX (((PAGE_SIZE >> BITS_PER_BYTE) \
                <= SLAB_OBJ_MIN_SIZE) ? 1 : 0)

#define cache_line_size()   64UL	

#if FREELIST_BYTE_INDEX
typedef unsigned char freelist_idx_t;
#else
typedef unsigned short freelist_idx_t;
#endif

#define SLAB_OBJ_MAX_NUM ((1 << sizeof(freelist_idx_t) * BITS_PER_BYTE) - 1)

#define MALLOC_MAX_ORDER 5

LIST_HEAD(slab_caches);

/*
 * Calculate the number of objects and left-over bytes for a given buffer size.
 */
static unsigned int mempool_estimate(unsigned long order, size_t buffer_size,
        unsigned long flags, size_t *left_over)
{
    unsigned int num;
    size_t slab_size = PAGE_SIZE << order;

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
    if (flags & (CFLGS_OBJFREELIST_SLAB | CFLGS_OFF_SLAB)) {
        num = slab_size / buffer_size;
        *left_over = slab_size % buffer_size;
    } else {
        num = slab_size / (buffer_size + sizeof(freelist_idx_t));
        *left_over = slab_size %
            (buffer_size + sizeof(freelist_idx_t));
    }

    return num;
}

/**
 * calculate_slab_order - calculate size (page order) of slabs
 * @mempool_p: pointer to the cache that is being created
 * @size: size of objects to be created in this cache.
 * @flags: slab allocation flags
 *
 * Also calculates the number of objects per slab.
 *
 * This could be made much more intelligent.  For now, try to avoid using
 * high order pages for slabs.  When the gfp() functions are more friendly
 * towards high-order requests, this should be changed.
 */
static size_t calculate_slab_order(struct mempool *mempool_p,
                size_t size, unsigned long flags)
{
    size_t left_over = 0;
    int order;

    for (order = 0; order <= MALLOC_MAX_ORDER; order++) {
        unsigned int num;
        size_t remainder;

        num = mempool_estimate(order, size, flags, &remainder);
        if (!num)
            continue;

        /* Can't handle number of objects more than SLAB_OBJ_MAX_NUM */
        if (num > SLAB_OBJ_MAX_NUM)
            break;

        /* Found something acceptable - save it away */
        mempool_p->num = num;
        mempool_p->order = order;
        left_over = remainder;

        /*
         * Acceptable internal fragmentation?
         */
        if (left_over * 8 <= (PAGE_SIZE << order))
            break;
    }
    return left_over;
}

static bool set_objfreelist_slab_cache(struct mempool *mempool_p,
            size_t size, unsigned long flags)
{
    size_t left;

    mempool_p->num = 0;

    if (mempool_p->ctor)
        return false;

    left = calculate_slab_order(mempool_p, size,
            flags | CFLGS_OBJFREELIST_SLAB);
    if (!mempool_p->num)
        return false;

    if (mempool_p->num * sizeof(freelist_idx_t) > mempool_p->object_size)
        return false;

    mempool_p->colour = left / mempool_p->colour_off;

    return true;
}

static bool set_off_slab_mempool(struct mempool *mempool_p,
            size_t size, unsigned long flags)
{
    size_t left;

    mempool_p->num = 0;

    /*
     * Size is large, assume best to place the slab management obj
     * off-slab (should allow better packing of objs).
     */
    left = calculate_slab_order(mempool_p, size, flags | CFLGS_OFF_SLAB);
    if (!mempool_p->num)
        return false;

    /*
     * If the slab has been placed off-slab, and we have enough space then
     * move it on-slab. This is at the expense of any extra colouring.
     */
    if (left >= mempool_p->num * sizeof(freelist_idx_t))
        return false;

    mempool_p->colour = left / mempool_p->colour_off;

    return true;
}

static bool set_on_slab_mempool(struct mempool *mempool_p,
            size_t size, unsigned long flags)
{
    size_t left;

    mempool_p->num = 0;

    left = calculate_slab_order(mempool_p, size, flags);
    if (!mempool_p->num)
        return false;

    mempool_p->colour = left / mempool_p->colour_off;

    return true;
}

/**
 * __mempool_create - Create a mempool.
 * @mempool_p: mempool management descriptor
 * @flags: memblock flags
 *
 * Returns a ptr to the mempool on success, NULL on failure.
 * Cannot be called within a int, but can be interrupted.
 * The @ctor is run when new pages are allocated by the cache.
 *
 * The flags are
 *
 * %SLAB_POISON - Poison the slab with a known test pattern (a5a5a5a5)
 * to catch references to uninitialised memory.
 *
 * %SLAB_RED_ZONE - Insert `Red' zones around the allocated memory to check
 * for buffer overruns.
 *
 * %SLAB_HWCACHE_ALIGN - Align the objects in this cache to a hardware
 * cacheline.  This can be beneficial if you're counting cycles as closely
 * as davem.
 */
int
__mempool_create(struct mempool *mempool_p, unsigned long flags)
{
    size_t ralign = BYTES_PER_WORD;
    int err;
    size_t size = mempool_p->size;

    /*
     * Check that size is in terms of words.  This is needed to avoid
     * unaligned accesses for some archs when redzoning is used, and makes
     * sure any on-slab bufctl's are also correctly aligned.
     */
    if (size & (BYTES_PER_WORD - 1)) {
        size += (BYTES_PER_WORD - 1);
        size &= ~(BYTES_PER_WORD - 1);
    }

    /* caller mandated alignment */
    if (ralign < mempool_p->align) {
        ralign = mempool_p->align;
    }
    /* disable debug if necessary */
    if (ralign > __alignof__(unsigned long long))
        flags &= ~(SLAB_RED_ZONE | SLAB_STORE_USER);
    /*
     * 4) Store it.
     */
    mempool_p->align = ralign;
    size = ALIGN(size, mempool_p->align);

    /*
     * We should restrict the number of objects in a slab to implement
     * byte sized index. Refer comment on SLAB_OBJ_MIN_SIZE definition.
     */
    if (FREELIST_BYTE_INDEX && size < SLAB_OBJ_MIN_SIZE)
        size = ALIGN(SLAB_OBJ_MIN_SIZE, mempool_p->align);

    if (set_objfreelist_slab_cache(mempool_p, size, flags)) {
        flags |= CFLGS_OBJFREELIST_SLAB;
        goto done;
    }

    if (set_off_slab_mempool(mempool_p, size, flags)) {
        flags |= CFLGS_OFF_SLAB;
        goto done;
    }

    if (set_on_slab_mempool(mempool_p, size, flags))
        goto done;

    return ERROR_TOOBIG;

done:
    mempool_p->freelist_size = mempool_p->num * sizeof(freelist_idx_t);
    mempool_p->flags = flags;
    mempool_p->size = size;

    if (OFF_SLAB(mempool_p)) {
        mempool_p->freelist_cache =
            calloc(mempool_p->freelist_size, 1);
    }

    return ERROR_SUCCESS;
}

static struct mempool* create_mempool(const char *name,
		size_t object_size, size_t size, size_t align,
		unsigned long flags, void (*ctor)(void *))
{
	struct mempool *s;
	int err;
	err = ERROR_NOMEM;

	s = calloc(1, sizeof(struct mempool));
	if (!s)
		goto out;

	s->name = name;
	s->object_size = object_size;
	s->size = size;
	s->align = align;
	s->ctor = ctor;

	err = __mempool_create(s, flags);
	if (err)
		goto out_free;

	s->refcount = 1;
	list_add(&s->list, &slab_caches);

out:
	return s;

out_free:
	free(s);
    s = NULL;
	goto out;
}

/*
 * Figure out what the alignment of the objects will be given a set of
 * flags, a user specified alignment and the size of the objects.
 */
unsigned long calculate_alignment(unsigned long flags,
		unsigned long align, unsigned long size)
{
	/*
	 * If the user wants hardware cache aligned objects then follow that
	 * suggestion if the object is sufficiently large.
	 *
	 * The hardware cache alignment cannot override the specified
	 * alignment though. If that is greater then use it.
	 */
	if (flags & SLAB_HWCACHE_ALIGN) {
		unsigned long ralign = cache_line_size();
		while (size <= ralign / 2)
			ralign /= 2;
		align = max(align, ralign);
	}

	return ALIGN(align, sizeof(void *));
}

struct mempool*
mempool_create(const char *name, size_t size, size_t align,
		  unsigned long flags, void (*ctor)(void *))
{
	/*
	 * Some allocators will constraint the set of valid flags to a subset
	 * of all flags. We expect them to define CACHE_CREATE_MASK in this
	 * case, and we'll just provide them with a sanitized version of the
	 * passed flags.
	 *
	flags &= CACHE_CREATE_MASK;*/

	return create_mempool(mempool_name, size, size,
			 calculate_alignment(flags, align, size),
			 flags, ctor);
}

static __always_inline void *
__do_cache_alloc(struct kmem_cache *cache, gfp_t flags)
{
    void *objp;

    if (current->mempolicy || cpuset_do_slab_mem_spread()) {
        objp = alternate_node_alloc(cache, flags);
        if (objp)
            goto out;
    }
    objp = ____cache_alloc(cache, flags);

    /*
     * We may just have run out of memory on the local node.
     * ____cache_alloc_node() knows how to locate memory on other nodes
     */
    if (!objp)
        objp = ____cache_alloc_node(cache, flags, numa_mem_id());

  out:
    return objp;
}

static __always_inline void *
slab_alloc(struct mempool *mempool_p, gfp_t flags)
{
    return __do_cache_alloc(mempool_p, flags);
}

/**
 * mempool_alloc - Allocate an object
 * @mempool_p: The cache to allocate from.
 * @flags: See kmalloc().
 *
 * Allocate an object from this cache.  The flags are only relevant
 * if the cache has no available objects.
 */
void *mempool_alloc(struct mempool *mempool_p, gfp_t flags)
{
    void *ret = slab_alloc(mempool_p, flags);

    /*kasan_slab_alloc(mempool_p, ret, flags);
    trace_mempool_alloc(_RET_IP_, ret,
                   mempool_p->object_size, mempool_p->size, flags);*/

    return ret;
}