#ifndef _WAIT_H
#define _WAIT_H

#ifdef __cplusplus
extern "C"{
#endif

#include "list.h"
#include "co_spinlock.h"
#include "coroutine_sched.h"

typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int flags, void *key);

/* wait_queue_entry::flags */
#define WQ_FLAG_EXCLUSIVE   0x01
#define WQ_FLAG_WOKEN       0x02
#define WQ_FLAG_BOOKMARK    0x04
#define WQ_FLAG_CUSTOM      0x08
#define WQ_FLAG_DONE        0x10

typedef struct wait_queue_head {
    co_spinlock_t spin_lock;
    struct list_head task_list;
} wait_queue_head_t;

typedef struct wait_queue_entry {
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

#define ___wait_is_interruptible(state)\
    (!__builtin_constant_p(state) ||\
        state == TASK_INTERRUPTIBLE || state == TASK_KILLABLE)\

#define ___wait_event(wq_head, condition, state, exclusive, ret, cmd)\
({                                                                   \
    __label__ __out;                                                 \
    struct wait_queue_entry __wq_entry;         \
    long __ret = ret;/* explicit shadow */      \
                                                \
    init_wait_entry(&__wq_entry, exclusive ? WQ_FLAG_EXCLUSIVE : 0);\
    for (;;) {\
        long __int = prepare_to_wait_event(&wq_head, &__wq_entry, state);\
                                        \
        if (condition)\ /*re check condition to avoid wakeup before.*/
            break;  \
                                        \
        if (___wait_is_interruptible(state) && __int) {\
            __ret = __int;\
            goto __out;\
        }\
                                        \
        cmd;\
    }\
    finish_wait(&wq_head, &__wq_entry);\
__out:__ret;\
})

#define __wait_event(wq_head, condition)					\
    (void)___wait_event(wq_head, condition, TASK_UNINTERRUPTIBLE, 0, 0,	\
                schedule())

#define wait_event(wq_head, condition)\
do {\
    if (condition)\
        break;\
    __wait_event(wq_head, condition);\
} while (0)

#define wake_up(x) __wake_up(x, CO_NORMAL, 1, NULL)


#ifdef __cplusplus
}
#endif

#endif