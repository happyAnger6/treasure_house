#include <assert.h>

#include "coroutine_sched.h"

static coroutine_t* co_queue_popleft(queue_t *queue)
{
    coroutine_t *co = NULL;
    pthread_mutex_lock(&queue->lock);
    if(list_empty(&queue->queue)) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    co = list_entry(queue->queue.next, coroutine_t, list);
    list_del(&co->list);
    pthread_mutex_unlock(&queue->lock);
    return co;
}

static int co_queue_append(queue_t *queue, coroutine_t *co)
{
    pthread_mutex_lock(&queue->lock);
    list_add_tail(&co->list, &queue->queue);
    pthread_mutex_unlock(&queue->lock);
    
    return 0;
}

static int pick_one_co(sched_t* sched)
{
    coroutine_t *co = co_queue_popleft(&sched->co_ready_queue);
    sched->co_curr = co;
    return co != NULL;
}

static void restore_co_stack(sched_t *sched, coroutine_t *co)
{
    memcpy(sched->stack + CO_STACK_SIZE - co->stack_size, co->stack, co->stack_size);
}

static void run_sched(sched_t* sched)
{
    coroutine_t *co = sched->co_curr;
    if(co->status != CO_READY)  // resched shoule restore stack first.
        restore_co_stack(co);
    
    co->status = CO_READY;
    swapcontext(&sched->uctx_main, &sched->co_curr->uctx);
}

static void main_loop(void *args)
{
    sched_t* sched = (sched_t *)args;
    while(sched->status == SCHED_RUNNING){
        if(pick_one_co(sched) == -1) {
            sched->status = SCHED_STOPPED;
            puts("all coroutines done!\r\n");
            break;
        }

        run_sched();
    }
}

sched_t* sched_create()
{
    sched_t *sched = (sched_t *)malloc(sizeof(sched_t));
    if(sched == NULL)
        error_exit("sched malloc failed!\r\n");

    queue_init(&sched->co_queue);
    queue_init(&sched->co_ready_queue);

    sched->co_curr = NULL;
    sched->status = SCHED_CREATED;

    return sched;
}

extern void sched_destory(sched_t *sched)
{
    free(sched);
}

extern void* sched_run(void *args)
{
    sched_t *sched = (sched_t *)args;
    sched->status = SCHED_RUNNING;
    main_loop(sched);
}

static void co_wrapper(sched_t *sched)
{
    coroutine_t *co = sched->co_curr;
    coroutine_func func = co->fn;
    func(co->fn_data);

    co_destory(co); // co already done.
}

static int make_co_context(sched_t *sched, coroutine_t *co)
{
    if(getcontext(&co->uctx) == -1)
        error_exit("make_co_context getcontext failed!");

    co->uctx.uc_stack.ss_sp = sched->stack;
    co->uctx.uc_stack.ss_size = sizeof(sched->stack);
    co->uctx.uc_link = &sched->uctx_main; //if end, return to sched's main_loop.
    makecontext(&co->uctx, co_wrapper, 1, sched);
    return 0;
}

static void save_co_stack(coroutine_t *co, char *top)
{
    char stack_base = 0;
    int32_t stack_size = top - &stack_base;
    assert(stack_size <= CO_STACK_SIZE);
    if(co->stack_size < stack_size) {
        free(co->stack); // free null is ok.
        co->stack = malloc(stack_size);
    }
    co->stack_size = stack_size;
    memcpy(co->stack, &stack_base, stack_size);
}

void sched_yield_coroutine(sched_t *sched)
{
    coroutine_t *co = sched->co_curr;
    co_queue_append(&sched->co_ready_queue, co);
    save_co_stack(co, sched->stack + sched->stack_size + CO_STACK_SIZE); 

    co->status = CO_YIELD;
    swapcontext(&co->uctx, &sched->uctx_main);
    return;
}

extern void sched_sched(sched_t *sched, coroutine_t *co)
{
    assert(co->status == CO_READY); 
    
    co_queue_append(&sched->co_ready_queue, co);
    make_co_context(sched, co); 
    co->sched = sched;

    return;
}