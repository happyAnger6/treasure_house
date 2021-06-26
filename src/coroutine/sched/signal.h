#ifndef _COROUTINE_SCHED_SIGNAL_H
#define _COROUTINE_SCHED_SIGNAL_H

#ifdef __cplusplus
extern "C"{
#endif

#include "coroutine.h"

static inline int signal_pending_state(long state, struct coroutine *co)
{
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif