#include <assert.h>

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

void processors_set_sched(sched_t *sched)
{
    if(pthread_getspecific(proc_key) == NULL)
        pthread_setspecific(proc_key, (void *)sched);
}

sched_t* processors_get_sched()
{
    if(pthread_getspecific(proc_key) == NULL)
        return NULL;
   
    return (sched_t *)pthread_getspecific(proc_key);
}

processors_t* processors_create()
{
    if(g_ps != NULL)
        return g_ps;

    int cpu_num = get_cpu_nums();
    g_ps->p_nums = cpu_num;
    g_ps->p_turn = 0;
    g_ps->all_p = malloc(cpu_num * sizeof(processor_t));

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
    sched_t *sched = processor_get_sched();
    assert(sched != NULL);

    int32_t delay = (int32_t)args;
    sched_delay(delay);
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


/* Wait for all processor_t in processors_t ended.*/
void processors_join()
{

}

void processors_submit(coroutine_t *co)
{
    processor_t *proc = NULL;
    proc = g_ps[g_ps->turn];
    g_ps->p_turn = (g_ps->p_turn + 1) % g_ps->p_nums;

    sched_sched(proc->sched, co);
}