#ifndef _CO_SPINLOCK_H
#define _CO_SPINLOCK_H

#ifdef __cplusplus
extern "C"{
#endif

#include "atm.h"

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

#ifdef __cplusplus
}
#endif

#endif
