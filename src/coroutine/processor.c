#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "threadpool.h"

#include "sync.h"
#include "processor.h"
#include "system_info.h"
#include "coroutine_types.h"
#include "coroutine_sched.h"
#include "future.h"

#define EXECUTOR_DEFAULT_NUM 2

static processors_t *g_ps;
static pthread_key_t proc_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;
static int g_maxprocs = -1;

static void make_proc_key()
{
    (void)pthread_key_create(&proc_key, NULL);
}

static void _add_wg(int val)
{
    wait_group_add(g_ps->wg, val);
}

static void _del_wg()
{
    wait_group_done(g_ps->wg);
}

void processors_set_sched(sched_t *sched)
{
    if(pthread_getspecific(proc_key) == NULL)
        pthread_setspecific(proc_key, (void *)sched);
}

int processors_set_maxprocs(int max_procs)
{
    if(g_maxprocs != -1)
        return -1;

    g_maxprocs = max_procs;
    return 0;
}

int processors_get_maxprocs()
{
    return g_maxprocs;
}

sched_t* processors_get_sched()
{
    void* sched = pthread_getspecific(proc_key);
    if (sched == NULL)
        return NULL;
   
    return (sched_t *)sched;
}

static void processors_destory()
{
    free(g_ps->all_p);
    free(g_ps);
    g_ps = NULL;
}

processors_t* processors_create()
{
    if(g_ps != NULL)
        return g_ps;

    int cpu_num = get_cpu_nums();
    if(g_maxprocs == -1)
        g_maxprocs = cpu_num;

    g_ps = (processors_t *)malloc(sizeof(processors_t));
    g_ps->p_nums = g_maxprocs;
    g_ps->p_turn = 0;
    g_ps->all_p = (processor_t *)malloc(g_maxprocs * sizeof(processor_t));
    g_ps->wg = wait_group_create();

    (void)pthread_once(&key_once, make_proc_key);

    int i = 0;
    for(i = 0; i < g_maxprocs; i++) {
        processor_t *proc = &g_ps->all_p[i];

        sched_t *sched = sched_create();
        if(sched == NULL)
            error_exit("sched creat failed!!!\r\n");
           
        proc->sched = sched;
        if(pthread_create(&proc->os_thread, NULL, sched_run, (void *)sched) != 0)
            error_exit("pthread create failed!!!\r\n");
    }

    g_ps->executor = threadpool_create(EXECUTOR_DEFAULT_NUM);
    return g_ps;
}

static void suspend_sleep(void *args)
{
    sched_t *sched = processors_get_sched();
    assert(sched != NULL);

    sched_delay(sched, (long)args);
}

void processors_suspend(suspend_type_t s_type, void *args)
{
    switch(s_type) {
        case SUSPEND_SLEEP:
            suspend_sleep(args);
            break;
        case SUSPEND_YIELD:
            sched_yield_coroutine(processors_get_sched());
            break;
        default:
            assert(0);
            break;
    }
}

static void exit_all_processors()
{
    processor_t *proc;
    for (int i = 0; i < g_ps->p_nums; i++)
    {
        proc = &g_ps->all_p[i];
        sched_wakeup(proc->sched);
        pthread_join(proc->os_thread, NULL);
    }
}

/* Wait for all processor_t in processors_t ended.*/
void processors_join()
{
    wait_group_wait(g_ps->wg);
    exit_all_processors();
    processors_destory();

    threadpool_shutdown(g_ps->executor, WAIT_ALL_DONE, NULL);
}

void processors_submit(coroutine_t *co)
{
    processor_t *proc = NULL;
    proc = &g_ps->all_p[g_ps->p_turn];
    g_ps->p_turn = (g_ps->p_turn + 1) % g_ps->p_nums;

    _add_wg(1);
    sched_sched(proc->sched, co);
}

void processors_coroutine_done()
{
    _del_wg();
}

typedef struct {
    future_t f;
    char *node;
    char *service;
    struct addrinfo *hints;
    struct addrinfo **res;
}getaddr_info_t;

static void* sync_getaddr_info(void *args)
{
    getaddr_info_t* getaddr_info = (getaddr_info_t *)args;
    future_t future = getaddr_info_t->f;

    getaddrinfo(getaddr_info->node, getaddr_info->service,
        getaddr_info->hints, getaddr_info->res);
    
    future_set_result(getaddr_info->res);
    return NULL;
}

ASYNC void* processors_getaddrinfo(const char *node, const char *service,
        const struct addrinfo *hints, struct addrinfo **res)
{
    future_t future = processors_create_co_future();
    struct task *getaddr_task = (struct task *)malloc(sizeof(struct task));
    getaddr_info_t *getaddr_info = (getaddr_info_t *)malloc(sizeof(getaddr_info_t));
    threadpool *executor = g_ps->executor;
    
    getaddr_info->f = future;
    getaddr_info->node = node;
    getaddr_info->service = service;
    getaddr_info->hints = hints;
    getaddr_info->res = res;

    getaddr_task->do_task = sync_getaddr_info;
    getaddr_task->args = (void *)getaddr_info;
    threadpool_submit(executor, getaddr_task);

    AWAIT future_await(future);
}

future_t processors_create_co_future()
{
    future_t future = future_create();
    sched_t *sched = processors_get_sched();
    future_add_done_callback(sched_co_resume, sched->co_curr);

    return future;
}

ASYNC void processors_await()
{
    sched_t *sched = processors_get_sched();
    sched_await_coroutine(sched);
}