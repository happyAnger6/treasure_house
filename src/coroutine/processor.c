#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sync.h"
#include "processor.h"
#include "system_info.h"
#include "coroutine_types.h"
#include "coroutine_sched.h"

static processors_t *g_ps;
static pthread_key_t proc_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

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
    g_ps = (processors_t *)malloc(sizeof(processors_t));
    g_ps->p_nums = cpu_num;
    g_ps->p_turn = 0;
    g_ps->all_p = (processor_t *)malloc(cpu_num * sizeof(processor_t));
    g_ps->wg = wait_group_create();

    (void)pthread_once(&key_once, make_proc_key);

    int i = 0;
    for(i = 0; i < cpu_num; i++) {
        processor_t *proc = &g_ps->all_p[i];

        sched_t *sched = sched_create();
        if(sched == NULL)
            error_exit("sched creat failed!!!\r\n");
           
        proc->sched = sched;
        if(pthread_create(&proc->os_thread, NULL, sched_run, (void *)sched) != 0)
            error_exit("pthread create failed!!!\r\n");
    }

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