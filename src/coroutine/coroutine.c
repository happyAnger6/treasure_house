#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "list.h"
#include "coroutine_types.h"
#include "coroutine.h"
#include "coroutine_sched.h"

static sched_t *sched_main;

static coroutine_t* _co_new(coroutine_func fn, void *args)
{
    coroutine_t *co = (coroutine_t *)malloc(sizeof(coroutine_t));
    if(co == NULL)
        error_exit("co new failed!");

    co->fn = fn;
    co->fn_data = args;
    co->stack = NULL;
    co->stack_size = 0;
    co->sched = NULL;
    co->status = CO_READY;
    co->stack = NULL;
    co->stack_size = 0;
    INIT_LIST_HEAD(&co->list);

    return co;
}

coroutine_t* coroutine_create(coroutine_func co_fn, void *args)
{
    coroutine_t *co =  _co_new(co_fn, args);
    sched_sched(sched_main, co);
    return co;
}

int coroutine_init()
{
    if(sched_main != NULL)
        error_exit("coroutine already inited!!!\r\n");

    sched_main = sched_create();
    return 0;
}

int coroutine_loop()
{
    if(sched_main == NULL) 
        error_exit("coroutine have not init yet!!!\r\n");

    sched_run((void *)sched_main);
    puts("coroutine loop exit.");
    return 0;
}

int coroutine_yield()
{
    return sched_yield(sched_main);
}

void co_destory(coroutine_t *co)
{
    assert(co != NULL);
    free(co);
}
