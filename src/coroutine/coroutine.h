#ifndef _COROUTINE_H
#define _COROUTINE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#include "list.h"

#include <ucontext.h>

typedef void (*coroutine_func)(void*);

typedef enum {
    CO_CREATE=0,
    CO_RUNNABLE,
    CO_RUNNINE,
    CO_WAITING,
    CO_END
} CO_STATUS;

typedef struct {
    void *sched;
    struct list_head list;
    ucontext_t uctx;
    char *stack;
    int32_t stack_size;
    void *fn_data;
    coroutine_func fn;
    CO_STATUS status;
} coroutine_t;

extern int coroutine_init();
extern coroutine_t* coroutine_create(coroutine_func co_fn, void *args);
extern int coroutine_yield();
extern int coroutine_loop();
extern void coroutine_destory(coroutine_t *co);

#ifdef __cplusplus
}
#endif

#endif