#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

#define HEAP_INIT_CAP 16

typedef struct {
    void **elements;
    heap_cmp_fn cmp_fn;
    uint32_t elem_size;
    uint32_t elem_num;
    uint32_t cap;
} _heap_t;

heap_t heap_create(heap_cmp_fn cmp_fn)
{
    _heap_t *ht = malloc(sizeof(_heap_t));

    ht->elem_size = sizeof(void *);
    ht->cap = HEAP_INIT_CAP;
    ht->elements = (void **)malloc(sizeof(void *) * HEAP_INIT_CAP);
    ht->elem_num = 0;
    ht->cmp_fn = cmp_fn;

    return (heap_t)ht;
}

static void sift_down(_heap_t * ht, int last)
{
    int parent_pos = -1;
    void *tmp = NULL;
    void **elements = ht->elements;
    while (last > 0) {
        parent_pos = (last - 1) >> 1;
        if (ht->cmp_fn(elements[parent_pos], elements[last]) <= 0)
            break;

        tmp = elements[last]; 
        elements[last] = elements[parent_pos];
        elements[parent_pos] = tmp;
        last = parent_pos;
    }
}

int heap_push(heap_t heap, void* elem)
{
    _heap_t *ht = heap;
    if (ht->cap == ht->elem_num) {
        ht->cap += HEAP_INIT_CAP;
        ht->elements = realloc(ht->elements, ht->elem_size * ht->cap);
    }

    ht->elements[ht->elem_num] = elem;
    ht->elem_num += 1;
    sift_down(ht, ht->elem_num - 1);
    return 0;
}

static void sift_up(_heap_t * ht, int last)
{
    void **elements = ht->elements;
    uint32_t size = ht->elem_size;
    if (last <= 0)
        return;
    
    elements[0] = elements[last];
    int parent_pos = 0; 
    int child_pos, right_pos;
    void *tmp;
    child_pos = (parent_pos << 1) + 1; 
    while (child_pos < last)
    {
        right_pos = child_pos + 1;
        if (right_pos < last && ht->cmp_fn(elements[right_pos], elements[child_pos]) < 0)
            child_pos = right_pos;

        if(ht->cmp_fn(elements[child_pos], elements[parent_pos]) >= 0)
            break;
        
        tmp = elements[child_pos]; 
        elements[child_pos] = elements[parent_pos];
        elements[parent_pos] = tmp;
        parent_pos = child_pos;
        child_pos = (child_pos << 1) + 1;
    }
}

void* heap_pop(heap_t heap)
{
    _heap_t *ht = heap;
    void *top;
    if (ht->elem_num == 0)
        return NULL;
    
    top = ht->elements[0];
    ht->elem_num -= 1;
    sift_up(ht, ht->elem_num);
    return top;
}

void* heap_top(heap_t heap)
{
    _heap_t *ht = (_heap_t *)heap;
    if (ht->elem_num == 0)
        return NULL;

    return ht->elements[0];
}

void heap_destory(heap_t heap)
{
    _heap_t *ht = (_heap_t *)heap;
    for(int i = 0; i < ht->elem_num; i++)
        free(ht->elements[i]);
    
    free(ht->elements);
}