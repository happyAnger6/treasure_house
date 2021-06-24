#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#ifdef __cplusplus
extern "C"{
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>

#include "threadpool.h"
#include "coroutine.h"
#include "coroutine_sched.h"
#include "sync.h"
#include "future.h"
#include "event_loop.h"

typedef struct {
    pthread_t os_thread;
    sched_t *sched;
} processor_t;

typedef struct {
    processor_t *all_p;
    struct threadpool *executor; //used for blocked API, for example, getaddrinfo.
    wait_group_t wg; // control all croutines
    int p_nums;
    int p_turn;
} processors_t;

typedef enum {
    SUSPEND_SLEEP = 0,
    SUSPEND_YIELD,
    SUSPEND_INVALID
} suspend_type_t;

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

extern int processors_set_maxprocs(int max_procs);

extern int processors_get_maxprocs();

/* Submit a coroutine to processors_t. processors_t will assign an
** approciate processor_t to coroutine.
*/
extern void processors_submit(coroutine_t *co);

/* Suspend current coroutine 
** s_type: suspend type. suspend_type_t
** args: args by suspend type.
*/
extern void processors_suspend(suspend_type_t s_type, void *args);

extern void processors_set_sched(sched_t *sched);

extern sched_t* processors_get_sched();

extern void processors_coroutine_done();

extern ASYNC int processors_getaddrinfo(const char *node, const char *service,
        const struct addrinfo *hints, struct addrinfo **res);

extern ASYNC int processors_tcp_connection(int sockfd, const struct sockaddr *addr,
        socklen_t addrlen);

extern future_t processors_create_co_future();

extern ASYNC void processors_await();

/* Return current event_loop*/
extern event_loop_t processors_get_event_loop();

#ifdef __cplusplus
}
#endif

#endif