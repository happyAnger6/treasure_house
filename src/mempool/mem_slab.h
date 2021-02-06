#ifndef _TH_MEMBLOCK_H
#define _TH_MEMBLOCK_H

/*
 * This restriction comes from byte sized index implementation.
 * Page size is normally 2^12 bytes and, in this case, if we want to use
 * byte sized index which can represent 2^8 entries, the size of the object
 * should be equal or greater to 2^12 / 2^8 = 2^4 = 16.
 * If minimum size of kmalloc is less than 16, we use it as minimum object
 * size and give up to use byte sized index.
 */
#define SLAB_OBJ_MIN_SIZE 16

struct page {
    /* First double word block */
    unsigned long flags;        /* Atomic flags, some possibly
    void *s_mem;            /* slab first object */
    void *freelist;     /* sl[aou]b first free object */
    unsigned int active;        /* SLAB */
            /*
             * Usage count, *USE WRAPPER FUNCTION* when manual
             * accounting. See page_ref.h
             */
    int _refcount;
    struct list_head lru;   /* Pageout list, eg. active_list
    struct page *next;  /* Next partial slab */
    short int pages;
    short int pobjects;

    struct kmem_cache *slab_cache;  /* SL[AU]B: Pointer to slab */
}

#endif