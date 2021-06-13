#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "co_time.h"
#include "coroutine_sched.h"
#include "event_loop.h"
#include "processor.h"

typedef void (*uctx_fn)();

#define lock_obj(obj) \
    pthread_mutex_lock(obj->lock)

#define unlock_obj(obj) \
    pthread_mutex_unlock(queue->lock)

/* Return a courinte_t* pop from queue header. If queue is empty,
** return NULL.
** 
** queue: a coroutine queue.
*/
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

/* Return 0 if append co to queue successfully.
** 
** queue: a coroutine queue.
** co: a coroutine to be added.
*/
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
    if(co->status != CO_RUNNABLE)  // resched shoule restore stack first.
        restore_co_stack(sched, co);
    
    co->status = CO_RUNNABLE;
    swapcontext(&sched->uctx_main, &sched->co_curr->uctx);
}

static long process_expired_timers(sched_t *sched)
{
    void *top;
    while ((top=heap_top(sched->co_timer_heap)) != NULL)
    {
        co_timer_t timer = (co_timer_t)top;
        time_now = co_timer_now();
        if (timer->when > time_now) 
            return timer->when - time_now;

        timer->callback(timer->args);
    }

    return -1;
}

static void main_loop(void *args)
{
    sched_t* sched = (sched_t *)args;
    while (sched->status == SCHED_RUNNING) {
        pthread_mutex_lock(&sched->lock);
        while(sched->co_nums == 0) 
        {
            if (sched->status != SCHED_RUNNING) {
                sched_destory(sched);
                return;
            }
            pthread_cond_wait(&sched->cond, &sched->lock);
        }
           
        if (pick_one_co(sched) != 0) {  // no ready coroutine means we should event loop wait for some event.
            pthread_mutex_unlock(&sched->lock);
            long delay = process_expired_timers(sched);
            event_loop_run(sched->ev_engine, delay);
            (void)process_expired_timers(sched);
        } else {
            pthread_mutex_unlock(&sched->lock);
            run_sched(sched);
        }
    }
}

sched_t* sched_create()
{
    sched_t *sched = (sched_t *)malloc(sizeof(sched_t));
    if(sched == NULL)
        error_exit("sched malloc failed!\r\n");

    pthread_mutex_init(&sched->lock, NULL);
    pthread_cond_init(&sched->cond, NULL);
    queue_init(&sched->co_queue);
    queue_init(&sched->co_ready_queue);

    sched->co_curr = NULL;
    sched->status = SCHED_CREATED;
    sched->stack_size = sizeof(sched->stack);
    sched->co_nums = 0;

    sched->co_timer_heap = heap_create(sizeof(void*), co_timer_cmp);
    sched->ev_engine = event_loop_create();
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

    processor_set_sched(sched); // set sched, so coroutine on sched can get it.
    main_loop(sched);
}

static void co_wrapper(sched_t *sched)
{
    coroutine_t *co = sched->co_curr;
    coroutine_func func = co->fn;
    func(co->fn_data);

    co_destory(co); // co already done.

    lock_obj(sched);
    sched->co_nums--;
    unlock_obj(sched);
}

static int make_co_context(sched_t *sched, coroutine_t *co)
{
    if(getcontext(&co->uctx) == -1)
        error_exit("make_co_context getcontext failed!");

    co->uctx.uc_stack.ss_sp = sched->stack;
    co->uctx.uc_stack.ss_size = sizeof(sched->stack);
    co->uctx.uc_link = &sched->uctx_main; //if end, return to sched's main_loop.
    makecontext(&co->uctx, (uctx_fn)co_wrapper, 1, sched);
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

static inline void co_schedule(sched_t *sched, coroutine_t *co, CO_STATUS status)
{
    save_co_stack(co, sched->stack + sched->stack_size); 
    co->status = status;
    swaptcontext(&co->uctx, &sched->uctx_main)
}

void sched_suspend_coroutine(sched_t *sched)
{
    co_queue_append(&sched->co_queue, co);
    co_schedule(sched, sched->co_curr, CO_WAITING);
}

void sched_yield_coroutine(sched_t *sched)
{
    co_queue_append(&sched->co_ready_queue, co);
    co_schedule(sched, sched->co_curr, CO_WAITING);
}

void sched_sched(sched_t *sched, coroutine_t *co)
{
    assert(co->status == CO_RUNNABLE); 

    make_co_context(sched, co); 
    co->sched = (void *)sched;
    
    // to avoid deadlock, we must keep locks sequence: sched->lock first, then queue->lock.
    pthread_mutex_lock(&sched->lock);     
    if(sched->co_nums == 0)
        pthread_cond_signal(&sched->cond);
    co_queue_append(&sched->co_ready_queue, co);
    sched->co_nums++;
    pthread_mutex_unlock(&sched->lock);

    return;
}

static void* co_after_delay(void* args)
{
    coroutine_t *co = (coroutine_t *)args;
    list_del(&co->list);

    co->status = CO_RUNNABLE;
    co_queue_append(&sched->co_ready_queue, co);
    return NULL;
}

void sched_delay(sched_t *sched, long delay_ms)
{
    coroutine_t *co = sched->co_curr;
    co_timer_t ct = co_timer_after(delay_ms, co_after_delay, (void *)co);
    heap_push(sched->co_timer_heap, (void *)ct);

    co->status = CO_WAITING;
    co_schedule(sched, sched->co_curr, CO_WAITING);
}