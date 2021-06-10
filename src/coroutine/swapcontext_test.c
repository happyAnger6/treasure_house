#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ucontext.h>

#define CO_STACK_SIZE 16*4096

static ucontext_t uctx_main, uctx_c1, uctx_c2;

#define error_exit(msg) \
    do { perror(msg); exit(-1); } while(0)

static void coroutine1()
{
    puts("coroutine1 begin...")
    if(swapcontext(&uctx_c1, &uctx_main) == -1) {
        error_exit("swapt c1 to main error.");
    }
    puts("coroutine1 end...")
}

static void coroutine2()
{
    puts("coroutine2 begin...")
    if(swapcontext(&uctx_c2, &uctx_main) == -1) {
        error_exit("swapt c2 to main error.");
    }
    puts("coroutine2 end...")
}

int main()
{
    char c1_stack[CO_STACK_SIZE];
    char c2_stack[CO_STACK_SIZE];

    if(getcontext(&uctx_c1) == -1)
        error_exit("getcontext c1");
    uctx_c1.uc_stack.ss_sp = c1_stack;
    uctx_c1.uc_stack.ss_size = sizeof(c1_stack);
    uctx_c1.uc_link = &uctx_main;
    makecontext(&uctx_c1, coroutine1, 0);

    if (getcontext(&uctx_c2) == -1)
        error_exit("getcontext c2");
    uctx_c2.uc_stack.ss_sp = c2_stack;
    uctx_c2.uc_stack.ss_size = sizeof(c2_stack);
    uctx_c2.uc_link = &uctx_main;
    makecontext(&uctx_c2, coroutine2, 0);

    int i = 0;
    for (i = 0; i < 4; i++ )
    {
        if (swapcontext(&uctx_main, &uctx_c1) == -1)
            error_exit("swapcontext");
        printf("        \e[35mmain: swapcontext(&uctx_main, &uctx_c2)\n\e[0m");

        if (swapcontext(&uctx_main, &uctx_c2) == -1)
            error_exit("swapcontext");
        printf("        \e[35mmain: swapcontext(&uctx_main, &uctx_c1)\n\e[0m");
    }

    printf("main: exiting\n");
    exit(0);
}