#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "list.h"
#include "coroutine_types.h"
#include "coroutine.h"
#include "processor.h"

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
    processors_submit( co);
    return co;
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

void coroutine_destory(coroutine_t *co)
{
    assert(co != NULL);
    free(co);
}