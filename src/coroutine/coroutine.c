#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

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

void idle()
{
    int i = 0;
    while(g_run_flag){
        sleep(1);

        i++;
        if(i == 10)
        {
            g_run_flag = 0;
            break;
        }

        puts("idle");

        if (swapcontext(&(main_sched->co_main->uctx), &uctx_main_loop) == -1)
            error_exit("main sched switch to loop error");
        
    }
}

static int _co_new(coroutine_func fn, void *args)
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
    co->uctx->uc_link = &main_sched->co_main->uctx; //if end, return to idle.
    if(makecontext(co->uctx, fn, 1, co->fn_data) == -1)
        error_exit("makecontext failed!");

    return 0;
}


int coroutine_create(coroutine_fn co_fn, void *args)
{
    return _co_new(fn, args);
}

int coroutine_init()
{
    if(main_sched != NULL) {
        error_exit("coroutine already inited\r\n");
    }

    main_sched = (sched_t *)malloc(sizeof(sched_t));
    if(main_sched == NULL)
        error_exit("malloc main sched_t failed\r\n");

    main_co = (coroutine_t *)malloc(sizeof(sched_t));
    if(main_co == NULL)
        error_exit("main_co malloc failed\r\n");

    char *main_co_stack = (char *)malloc(CO_STACK_SIZE);
    if(main_co_stack == NULL)
        error_exit("main_co_stack malloc failed\r\n");
    
    if(getcontext(&main_co->uctx) == -1)
        error_exit("malloc main sched_t failed\r\n");
    ucontext_t *uctx_main_co = &main_co->uctx;
    uctx_main_co->uc_stack.ss_sp = main_co_stack;
    uctx_main_co->uc_stack.ss_size = CO_STACK_SIZE;
    uctx_main_co->uc_link = &uctx_main_loop;
    makecontext(uctx_main_co, idle, 0);

    main_sched->co_main = main_co;
    main_sched->co_curr = main_co;
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

int coroutine_sched()
{
    if(swapcontext(&main_sched->co_curr->uctx, &main_sched->co_main->uctx)==-1)
        error_exit("sched swapcontext failed!");
    
    return 0;
}

#ifdef TEST_MAIN

int main()
{
    coroutine_init();

    return coroutine_loop();
}

#endif