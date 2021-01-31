#include "treasure_house/list.h"
#include "treasure_house/mempool.h"

typedef unsigned char freelist_idx_t;

static void* th_mempool_grow(th_mempool_t *mempool_p);

static bool set_on_block_pool(th_mempool_t *mempool_p,
			size_t size, unsigned long flags);

static size_t caculate_block_order(th_mempool_t *mempool_p,
			size_t size, unsigned long flags);

static unsigned int block_estimate(unsigned long order,
		unsigned long flags, size_t *left_over);
