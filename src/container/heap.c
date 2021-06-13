#include "heap.h"

typedef struct {
    void *elements;
    heap_cmp_fn cmp_fn;
    uint32_t elem_size;
    uint32_t elem_num;
} heap_t;

