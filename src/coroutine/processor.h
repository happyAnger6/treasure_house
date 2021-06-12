#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#include <pthread.h>

#include "coroutine.h"
#include "coroutine_sched.h"

typedef struct {
    pthread_t os_thread;
    sched_t *sched;
} processor_t;

typedef struct {
    processor_t *all_p;
    int p_nums;
} processors_t;

/* Create a processors for execute coroutines.
**
** processors_t is consists of processor_t which uses an os thread 
**  and represent a logic cpu. The nums of processor_t is equal to 
**  cpu cores. Every processor_t have a local queue consist of coroutines
**  to be executed in it.
*/
extern processors_t* processors_create();

/* Wait for all processor_t in processors_t ended.*/
extern void processors_join();

/* Submit a coroutine to processors_t. processors_t will assign an
** approciate processor_t to coroutine.
*/
extern void processors_submit(coroutine_t *co);
#endif