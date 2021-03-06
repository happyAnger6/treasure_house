#ifndef _COROUTINE_SCHED_H
#define _COROUTINE_SCHED_H

#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>

#include <ucontext.h>
#include <pthread.h>

#include "heap.h"
#include "coroutine_types.h"
#include "coroutine.h"
#include "event_loop.h"
#include "wait.h"

#define CO_STACK_SIZE 1024*1024  // 1M is shared by all coroutines in this sched.

#define SCHED_CREATED 0
#define SCHED_RUNNING 1
#define SCHED_STOPPED 2

typedef struct queue {
    pthread_mutex_t lock;
    struct list_head queue;
} *queue_t;

static inline int queue_init(struct queue *q)
{
    INIT_LIST_HEAD(&q->queue);
    return pthread_mutex_init(&q->lock, NULL);
}

typedef struct sched {
	pthread_cond_t cond;
	pthread_mutex_t lock;
    queue_t co_queue;
    queue_t co_ready_queue;
    heap_t co_timer_heap;
    struct coroutine *co_curr;
    ucontext_t uctx_main;
    event_loop_t ev_engine;
    int status;
	int co_nums;
    int32_t stack_size;
    char stack[CO_STACK_SIZE]; // shared by all coroutines in this sched_t.
} *sched_t;

extern sched_t sched_create();
extern void sched_destory(sched_t sched);
extern void* sched_run_entry(void *args);
extern void sched_sched(sched_t sched, struct coroutine *co);
extern void sched_yield_coroutine(sched_t sched);
extern void sched_stop(sched_t sched);
extern void sched_wakeup(sched_t sched);
extern void sched_await_coroutine(sched_t sched);

/* resume a co because co is reReady.*/
extern void* sched_co_resume(void *args);

extern int32_t sched_coroutine_nums(sched_t sched);

/* Delay current running coroutine delay_ms ms.*/
extern void sched_delay(sched_t sched, long delay_ms);

extern coroutine_t sched_current_coroutine(sched_t sched);

extern int sched_wakeup_coroutine(wait_queue_t *curr, int wake_flags);
int autoremove_wake_function(wait_queue_t *wait, int wake_flags);

/*asyncio lib api*/
#ifdef __cplusplus
}
#endif

#endif
