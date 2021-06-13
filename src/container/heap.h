#ifndef _HEAP_H
#define _HEAP_H 

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

typedef int (*heap_cmp_fn)(void* elem1, void* elem2);

typedef void* heap_t;

extern heap_t heap_create(uint32_t elem_size, heap_cmp_fn cmp_fn);
extern int heap_push(heap_t heap, void* elem);
extern void* heap_pop(heap_t heap);
extern void* heap_top(heap_t heap);

#ifdef __cplusplus
}
#endif

#endif