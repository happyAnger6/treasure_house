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
    DEFINE_WAIT(wait);

    while (!atm_rel_cas(&mutex->used, 0, 1)) {

        /* wait and unlock must in guard to avoid race when lock failed*/
        co_spin_lock(&mutex->guard);
        if (!atm_rel_cas(&mutex->used, 0, 1)) { /* coroutine unlocked during time window*/
            co_spin_unlock(&mutex->guard);
            break;
        }

        prepare_to_wait(&mutex->wq, &wait, CO_UNINTERRUPTIBLE);
        co_spin_unlock(&mutex->guard);

        /* 
         * during schedule another courinte_t may acquire mutex, 
         * so when wakeup must recheck mutex->used
         */
        sched_schedule(); 
    }

    finish_wait(&mutex->wq, &wait);
    return 0;
}

int co_mutex_unlock(co_mutex_t *mutex)
{
    /* unlock and wait must in guard to avoid race when lock failed*/
    co_spin_lock(&mutex->guard);
    
    atm_rel_store(&mutex->used, 0);

    /* 
     * during wakeup another coroutine may acquire mutex_t, 
     * so coroutine be wakeuped must recheck mutex
     */
    wake_up(&mutex->wq);

    co_spin_unlock(&mutex->guard);
    return 0;
}