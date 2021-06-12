#ifndef _COROUTINE_SCHED_H
#define _COROUTINE_SCHED_H

#ifdef __cplusplus
extern "C"{
#endif

#include <ucontext.h>
#include <pthread.h>

#include "coroutine_types.h"
#include "coroutine.h"

#define CO_STACK_SIZE 1024*1024  // 1M is shared by all coroutines in this sched.

#define SCHED_CREATED 0
#define SCHED_RUNNING 1
#define SCHED_STOPPED 2

typedef struct {
    pthread_mutex_t lock;
    struct list_head queue;
} queue_t;

static inline int queue_init(queue_t *q)
{
    INIT_LIST_HEAD(&q->queue);
    return pthread_mutex_init(&q->lock);
}

typedef struct {
    int status;
    queue_t co_queue;
    queue_t co_ready_queue;
    char stack[CO_STACK_SIZE]; // shared by all coroutines in this sched_t.
    ucontext_t uctx_main;
    coroutine_t *co_curr;
} sched_t;

extern sched_t* sched_create();
extern void sched_destory(sched_t *sched);
extern void sched_run(sched_t *sched);
extern void sched_sched(coroutine_t *co);
extern void sched_yield(coroutine_t *co);
extern void sched_stop(sched_t *sched);


#ifdef __cplusplus
}
#endif

#endif
