#ifndef _WAIT_H
#define _WAIT_H

#ifdef __cplusplus
extern "C"{
#endif

#include "list.h"
#include "co_spinlock.h"
#include "coroutine_sched.h"

typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int flags, void *key);

typedef struct wait_queue_head {
    co_spinlock_t spin_lock;
    struct list_head task_list;
} wait_queue_head_t;

typedef struct wait_queue {
    unsigned int flag;
    void *private; // coroutine_t which waited to be wakeup.
    wait_queue_func_t func;
    struct list_head task_list;
} wait_queue_t;

#define DEFINE_WAIT(name) \
    wait_queue_t name = { \
        .private = coroutine_current(), \
        .func = autoremove_wake_function, \
        .task_list = LIST_HEAD_INIT((name).task_list), \
    }


static inline void _add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
    list_add(&new->task_list, &head->task_list);
}

static inline void _add_wait_queue_tail(wait_queue_head_t *head, wait_queue_t *new)
{
    list_add_tail(&new->task_list, &head->task_list);
}

extern void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state);
extern void init_wait_entry(wait_queue_t *wait, int flags);
extern void finish_wait(wait_queue_head_t *q, wait_queue_t *wait);

#ifdef __cplusplus
}
#endif

#endif