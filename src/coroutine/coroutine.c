#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "list.h"
#include "coroutine_types.h"
#include "coroutine.h"
#include "processor.h"
#include "wait.h"

static coroutine_t _co_new(coroutine_func fn, void *args)
{
    coroutine_t co = (coroutine_t )malloc(sizeof(struct coroutine));
    if(co == NULL)
        error_exit("co new failed!");

    co->fn = fn;
    co->fn_data = args;
    co->stack = NULL;
    co->stack_size = 0;
    co->sched = NULL;
    co->status = CO_RUNNABLE;
    co->stack = NULL;
    co->stack_size = 0;
    INIT_LIST_HEAD(&co->list);

    return co;
}

coroutine_t coroutine_create(coroutine_func co_fn, void *args)
{
    coroutine_t co =  _co_new(co_fn, args);
    processors_submit(co);
    return co;
}

coroutine_t coroutine_current()
{
    sched_t cur_sched = processors_get_sched();
    return sched_current_coroutine(cur_sched);
}

void coroutine_set_current_state(int state)
{
    sched_t cur_sched = processors_get_sched();
    coroutine_t co = sched_current_coroutine(cur_sched);
    co->status = state;
    if (state != CO_RUNNABLE)
        list_del(co->list); // remove from ready_queue;
}

int coroutine_init()
{
    processors_create();
    return 0;
}

int coroutine_loop()
{
    processors_join();
    puts("coroutine loop exit.");
    return 0;
}

int coroutine_yield()
{
    processors_suspend(SUSPEND_YIELD, NULL);
    return 0;
}

void coroutine_destory(coroutine_t co)
{
    assert(co != NULL);
    free(co);
}