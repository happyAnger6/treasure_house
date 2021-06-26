#ifndef _COROUTINE_H
#define _COROUTINE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#include "list.h"
#include "wait.h"

#include <ucontext.h>

typedef void (*coroutine_func)(void*);

typedef enum {
    CO_CREATE=0,
    CO_RUNNABLE,
    CO_RUNNINE,
    CO_WAITING,
    CO_INTERRUPTIBLE,
    CO_UNINTERRUPTIBLE,
    CO_END
} CO_STATUS;

/* Convenience macros for the sake of wake_up(): */
#define CO_NORMAL (CO_INTERRUPTIBLE | CO_UNINTERRUPTIBLE)

typedef struct coroutine {
    void *sched;
    struct list_head list;
    ucontext_t uctx;
    char *stack;
    int32_t stack_size;
    void *fn_data;
    coroutine_func fn;
    CO_STATUS status;
} *coroutine_t;

extern int coroutine_init();
extern coroutine_t coroutine_create(coroutine_func co_fn, void *args);
extern int coroutine_yield();
extern int coroutine_loop();
extern void coroutine_destory(coroutine_t co);
extern void coroutine_await(future_t future);

/* api control current coroutine, like linux kernel current.
 * todo: optimize current use thread local data.
*/
extern coroutine_t coroutine_current();
void coroutine_set_current_state(int state);

#ifdef __cplusplus
}
#endif

#endif