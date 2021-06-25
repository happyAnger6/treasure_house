#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "atm.h"

#include "coroutine.h"

typedef volatile int co_spinlock_t;

int co_spin_init(co_spinlock_t *lock)
{
    atm_rel_store(lock, 0);
    return 0;
}

int co_spin_lock(co_spinlock_t *lock)
{
    return atm_full_xchg(&lock, 0, 1);
}

int co_spin_unlock(co_spinlock_t *lock)
{
    atm_rel_store(lock, 0);
    return 0;
}

int co_spin_destory(co_spinlock_t *lock)
{
    return 0;
}

typedef struct {
    co_spinlock_t guard;
    volatile int used;
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
    int cur = atm_acq_load(&mutex->used);
    if (atm_rel_cas(&mutex->used, cur, 1))
        return 0;

    /* wait and unlock must in guard to avoid race when lock failed*/
    co_spin_lock(&mutex->guard);

    co_spin_unlock(&mutex->guard);
}

int co_mutex_unlock(co_mutex_t *mutex)
{
    /* unlock and wait must in guard to avoid race when lock failed*/
    co_spin_lock(&mutex->guard);
    
    co_spin_unlock(&mutex->guard);
}