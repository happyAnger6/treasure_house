#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "list.h"
#include "coroutine.h"

typedef enum {
    CO_CREATE=0,
    CO_READY,
    CO_RUNNINE,
    CO_SUSPEND,
    CO_END
}CO_STATUS;

typedef struct {
    struct list_head list;
    ucontext_t uctx;
    void *fn_data;
    coroutine_func fn;
    CO_STATUS status;
}coroutine_t;

typedef struct {
    struct list_head co_ready_list;
    coroutine_t *co_main;
    coroutine_t *co_curr;
}sched_t;

static sched_t *sched_main;
static coroutine_t *co_main;
static ucontext_t uctx_main_loop;

static int g_run_flag = 1;

static inline void error_exit(char *msg)
{
    perror(msg);
    exit(1);
}

static void co_wrapper()
{
    coroutine_t *co = sched_main->co_curr;
    coroutine_func func = co->fn;
    func(co->fn_data);

    co_destory(co); // co already done.
}

int run_sched()
{
    return swapcontext(&sched_main->co_main->uctx, &sched_main->co_curr->uctx);
}

int select_one_co()
{
    coroutine_t *co;
    if(list_empty(&sched_main->co_ready_list)) {
        sched_main->co_curr = NULL; // no ready, go to main loop
        return -1;
    }

    co = list_entry(sched_main->co_ready_list.next, coroutine_t, list);
    list_del(&co->list);
    sched_main->co_curr = co;
    return 0;
}

void idle(void *args)
{
    while(g_run_flag){
        if(select_one_co() == -1) {
            g_run_flag = 0;
            puts("all coroutines done!\r\n");
            break;
        }

        run_sched();
    }
}

static coroutine_t* _co_new(coroutine_func fn, void *args, int main_flag)
{
    coroutine_t *co = (coroutine_t *)malloc(sizeof(coroutine_t));
    if(co == NULL)
        error_exit("co new failed!");

    co->fn = fn;
    co->fn_data = args;
    co->status = CO_READY;
    INIT_LIST_HEAD(&co->list);

    if(getcontext(&co->uctx) == -1)
        error_exit("getcontext failed!");

    co->uctx.uc_stack.ss_sp = malloc(CO_STACK_SIZE);
    if(co->uctx.uc_stack.ss_sp == NULL)
        error_exit("co uctx malloc stack failed!");
    co->uctx.uc_stack.ss_size = CO_STACK_SIZE;
    co->uctx.uc_link = &uctx_main_loop; //if end, return to main_loop.
    makecontext(&co->uctx, co_wrapper, 0);

    if(main_flag == 0)
        list_add_tail(&co->list, &sched_main->co_ready_list);

    return co;
}

coroutine_handle coroutine_create(coroutine_func co_fn, void *args)
{
    return (coroutine_handle)_co_new(co_fn, args, 0);
}

int coroutine_init()
{
    if(sched_main != NULL) {
        error_exit("coroutine already inited\r\n");
    }

    sched_main = (sched_t *)malloc(sizeof(sched_t));
    if(sched_main == NULL)
        error_exit("malloc main sched_t failed\r\n");

    INIT_LIST_HEAD(&sched_main->co_ready_list);
    
    co_main = _co_new(idle, NULL, 1);
    sched_main->co_main = co_main;
    sched_main->co_curr = co_main;

    return 0;
}

int coroutine_loop()
{
    while(g_run_flag) {
        if (swapcontext(&uctx_main_loop, &sched_main->co_main->uctx) == -1)
            error_exit("coroutine_loop swapcontext error");
    }
    
    puts("coroutine loop exit.");
    return 0;
}

int coroutine_yield()
{
    coroutine_t *co = sched_main->co_curr;
    list_add_tail(&co->list, &sched_main->co_ready_list);  // resched later.

    if(swapcontext(&sched_main->co_curr->uctx, &sched_main->co_main->uctx)==-1)
        error_exit("sched swapcontext failed!");
    
    return 0;
}

void co_destory(coroutine_t *co)
{
    assert(co != NULL);
}
