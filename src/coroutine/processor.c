#include "processor.h"
#include "system_info.h"
#include "coroutine_types.h"
#include "coroutine_sched.h"

static processors_t *g_ps;

processors_t* processors_create()
{
    if(g_ps)
        return g_ps;

    int cpu_num = get_cpu_nums();
    g_ps->p_nums = cpu_num;
    g_ps->all_p = malloc(cpu_num * sizeof(processor_t));

    int i = 0;
    for(i = 0; i < cpu_num; i++) {
        processor_t *proc = g_ps->all_p[i];

        sched_t *sched = sched_create();
        if(sched == NULL)
            error_exit("sched creat failed!!!\r\n");
           
        proc->sched = sched;
        if(pthread_create(&proc->os_thread, NULL, sched->sched_run, sched) != 0)
            error_exit("pthread create failed!!!\r\n");
    }

    return g_ps;
}

/* Wait for all processor_t in processors_t ended.*/
extern void processors_join();

/* Submit a coroutine to processors_t. processors_t will assign an
** approciate processor_t to coroutine.
*/
extern void processors_submit(coroutine_t *co)