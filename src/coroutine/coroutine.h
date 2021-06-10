#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <ucontext.h>

#define CO_STACK_SIZE 16*1024

typedef void (*coroutine_func)(void*);

typedef enum {
    CO_CREATE=0,
<<<<<<< HEAD
    CO_READY,
    CO_RUNNINE,
    CO_SUSPEND,
    CO_END
=======
    CO_RUNNINE=1,
    CO_SUSPEND=2,
    CO_END=3
>>>>>>> 48895903c642875dfee71baa52a3e1cb3883909e
}CO_STATUS;

typedef struct {
    ucontext_t uctx;
    void *fn_data;
    coroutine_func fn;
    CO_STATUS status;
}coroutine_t;

typedef struct {
    coroutine_t *co_main;
    coroutine_t *co_curr;
}sched_t;


extern int coroutine_init();
extern int coroutine_loop();
extern int coroutine_sched();

#endif