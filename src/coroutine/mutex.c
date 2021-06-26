#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "atm.h"

#include "coroutine.h"
#include "co_spinlock.h"
#include "wait.h"

typedef struct {
    co_spinlock_t guard;
    volatile int used;
    wait_queue_head_t wq;
    struct list_head waiters;
} co_mutex_t;

int co_mutex_init(co_mutex_t *mutex)
{
    co_spin_init(&mutex->guard);
    mutex->used = 0;
    INIT_LIST_HEAD(&mutex->waiters);

    return 0;
}

int co_mutex_lock(co_mutex_t *mutex)
{
    wait_event(&mutex->wq, atm_rel_cas(&mutex->used, 0, 1));

    return 0;
}

int co_mutex_unlock(co_mutex_t *mutex)
{
    atm_rel_store(&mutex->used, 0);
    wake_up(&mutex->wq);
    
    return 0;
}