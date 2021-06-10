#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "list.h"
#include "coroutine.h"

static sched_t *main_sched;
static coroutine_t *main_co;
static ucontext_t uctx_main_loop;

static int g_run_flag = 1;

static inline void error_exit(char *msg)
{
    perror(msg);
    exit(1);
}

static void co_wrapper()
{
    coroutine_t *co = main_sched->co_curr;
    coroutine_fn = co->fn;
    coroutine_fn(co->fn_data);

    co_destory(co); // co already done.
}

void run_sched()
{
    return swapcontext(&main_sched->co_main->uctx, &main_sched->co_curr->uctx);
}

int select_one_co()
{
    coroutine_t *co;
    if(list_empty(&main_sched->co_ready_list)) {
        main_sched->co_curr = NULL; // no ready, go to main loop
        return -1;
    }

    co = list_entry(main_sched->co_ready_list.next, coroutine_t, list);
    list_del(&co->list);
    main_sched->co_curr = co;
    return 0;
}

void idle(void *args)
{
    while(g_run_flag){
        if(select_one_co() == -1) {
            g_run_flag = 0;
            puts("all coroutines done!\r\n")
            break;
        }

        run_sched();
    }
}

static coroutine_t* _co_new(coroutine_func fn, void *args, int main_flag)
{
    coroutine_t co = (coroutine_t *)malloc(sizeof(coroutine_t));
    if(co == NULL)
        error_exit("co new failed!");

    co->fn = fn;
    co->fn_data = args;
    co->status = CO_READY;

    co->uctx = (ucontext_t *)malloc(sizeof(ucontext_t));
    if(co->uctx == NULL)
        error_exit("co uctx malloc failed!");

    if(getcontext(co->uctx) == -1)
        error_exit("getcontext failed!");

    co->uctx->uc_stack.ss_sp = malloc(CO_STACK_SIZE);
    if(co->uctx->uc_stack.ss_sp == NULL)
        error_exit("co uctx malloc stack failed!");
    co->uctx->uc_stack.ss_size = CO_STACK_SIZE;
    co->uctx->uc_link = &uctx_main_loop; //if end, return to main_loop.
    if(makecontext(co->uctx, co_wrapper, 0) == -1)
        error_exit("makecontext failed!");

    if(main_flag == 0)
        list_add_tail(&main_sched->co_ready_list, &co->list);

    return co;
}

coroutine_t* coroutine_create(coroutine_fn co_fn, void *args)
{
    return _co_new(fn, args, 0);
}

int coroutine_init()
{
    if(main_sched != NULL) {
        error_exit("coroutine already inited\r\n");
    }

    main_sched = (sched_t *)malloc(sizeof(sched_t));
    if(main_sched == NULL)
        error_exit("malloc main sched_t failed\r\n");

    main_co = _co_new(idle, NULL, 1);

    main_sched->co_main = main_co;
    main_sched->co_curr = main_co;

    INIT_LIST_HEAD(&main_sched->co_ready_list);
    return 0;
}

int coroutine_loop()
{
    while(g_run_flag) {
        if (swapcontext(&uctx_main_loop, &main_sched->co_main->uctx) == -1)
            error_exit("coroutine_loop swapcontext error");
    }
    
    puts("coroutine loop exit.");
    return 0;
}

int coroutine_yield()
{
    coroutine_t *co = main_sched->co_curr;
    list_add_tail(&co->list, &main_sched->co_ready_list);  // resched later.

    if(swapcontext(&main_sched->co_curr->uctx, &main_sched->co_main->uctx)==-1)
        error_exit("sched swapcontext failed!");
    
    return 0;
}

void co_destory(coroutine_t *co)
{
    assert(co != NULL);
}

#ifdef TEST_MAIN

void co1(void *args)
{
    puts("co1 begin...")
    coroutine_yield();
    puts("co1 end...")
}

void co2(void *args)
{
    puts("co2 begin...")
    coroutine_yield();
    puts("co2 end...")
}

int main()
{
    coroutine_init();
    coroutine_create(co1, NULL);
    coroutine_create(co2, NULL);

    return coroutine_loop();
}

#endif