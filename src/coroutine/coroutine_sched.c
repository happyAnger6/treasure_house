#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>

#include "co_time.h"
#include "coroutine_sched.h"
#include "event_loop.h"
#include "processor.h"

typedef void (*uctx_fn)();

#define lock_obj(obj) \
    pthread_mutex_lock(&obj->lock)

#define unlock_obj(obj) \
    pthread_mutex_unlock(&obj->lock)

/* Return a courinte_t* pop from queue header. If queue is empty,
** return NULL.
** Caution: should hold sched lock first.
**
** queue: a coroutine queue.
*/
static inline struct coroutine* co_queue_popleft(struct queue *queue)
{
    struct coroutine *co = NULL;
    if(list_empty(&queue->queue)) {
        return NULL;
    }

    co = list_entry(queue->queue.next, struct coroutine, list);
    list_del(&co->list);
    return co;
}

/* Return 0 if append co to queue successfully.
** 
** queue: a coroutine queue.
** co: a coroutine to be added.
*/
static inline int co_queue_append(struct queue *queue, struct coroutine *co)
{
    list_add_tail(&co->list, &queue->queue);
    return 0;
}

static inline int pick_one_co(sched_t sched)
{
    lock_obj(sched);
    struct coroutine *co = co_queue_popleft(&sched->co_ready_queue);
    unlock_obj(sched);
    sched->co_curr = co;
    return co == NULL;
}

static void save_co_stack(struct coroutine *co, char *top)
{
    char stack_base = 0;
    int32_t stack_size = top - &stack_base;
    assert(stack_size <= CO_STACK_SIZE);

    if(co->stack_size < stack_size) {
        free((void *)co->stack); // free null is ok.
        co->stack = (char *)malloc(stack_size);
    }

    co->stack_size = stack_size;
    memcpy(co->stack, &stack_base, stack_size);
}

static inline void restore_co_stack(sched_t sched, struct coroutine *co)
{
    memcpy(sched->stack + CO_STACK_SIZE - co->stack_size, co->stack, co->stack_size);
}

static void schedule(struct sched* sched)
{
    struct coroutine *co = sched->co_curr;
    if(co->status != CO_RUNNABLE)  // resched shoule restore stack first.
        restore_co_stack(sched, co);
    
    co->status = CO_RUNNABLE;
    swapcontext(&sched->uctx_main, &sched->co_curr->uctx);
}

static void sched_exit(struct sched *sched)
{
    heap_destory(sched->co_timer_heap);
    event_loop_destory(sched->ev_engine);
    free(sched);
}

static long process_expired_timers(struct sched *sched)
{
    void *top;
    while ((top=heap_top(sched->co_timer_heap)) != NULL) {
        co_timer_t timer = (co_timer_t)top;
        long time_now = co_timer_now();
        if (timer->when > time_now) 
            return timer->when - time_now;

        heap_pop(sched->co_timer_heap);
        co_timer_run(timer);
        co_timer_destory(timer);
    }

    return 0; // if no timers return 0, event_loop will return immediately.
}

static void main_loop(struct sched *sched)
{
    while (sched->status == SCHED_RUNNING) {
        lock_obj(sched);
        while(sched->co_nums == 0) {
            if (sched->status != SCHED_RUNNING) {
                unlock_obj(sched);
                return;
            }
            pthread_cond_wait(&sched->cond, &sched->lock); // wait for new coroutines.
        }

        unlock_obj(sched);
        if (pick_one_co(sched) != 0) // no ready coroutine means we should event loop wait for some event.
        {   
            long delay = process_expired_timers(sched);
            event_loop_run(sched->ev_engine, delay);
            (void)process_expired_timers(sched);
        } 
        else 
        {
            schedule(sched);
        }
    }
}

sched_t sched_create()
{
    sched_t sched = (sched_t)malloc(sizeof(struct sched));
    if(sched == NULL)
        error_exit("sched malloc failed!\r\n");

    pthread_mutex_init(&sched->lock, NULL);
    pthread_cond_init(&sched->cond, NULL);
    queue_init(sched->co_queue);
    queue_init(sched->co_ready_queue);

    sched->co_curr = NULL;
    sched->status = SCHED_CREATED;
    sched->stack_size = sizeof(sched->stack);
    sched->co_nums = 0;

    sched->co_timer_heap = heap_create(co_timer_cmp);
    sched->ev_engine = event_loop_create();
    return sched;
}

void* sched_run_entry(void *args)
{
    sched_t sched = (sched_t)args;
    sched->status = SCHED_RUNNING;

    processors_set_sched(sched); // set sched, so coroutine on sched can get it.
    main_loop(sched);
    sched_exit(sched);
}

static void co_wrapper(sched_t sched)
{
    struct coroutine *co = sched->co_curr;
    coroutine_func func = co->fn;
    func(co->fn_data);

    coroutine_destory(co); // co already done.

    lock_obj(sched);
    sched->co_nums--;
    unlock_obj(sched);

    processors_coroutine_done();
}

static int make_co_context(sched_t sched, struct coroutine *co)
{
    if(getcontext(&co->uctx) == -1)
        error_exit("make_co_context getcontext failed!");

    co->uctx.uc_stack.ss_sp = sched->stack;
    co->uctx.uc_stack.ss_size = sizeof(sched->stack);
    co->uctx.uc_link = &sched->uctx_main; //if end, return to sched's main_loop.
    makecontext(&co->uctx, (uctx_fn)co_wrapper, 1, sched);
    return 0;
}

static inline void schedule_yield_coroutine(sched_t sched, struct coroutine *co,
                                CO_STATUS status)
{
    save_co_stack(co, sched->stack + sched->stack_size); 
    co->status = status;
    swapcontext(&co->uctx, &sched->uctx_main);
}

void sched_suspend_coroutine(sched_t sched)
{
    lock_obj(sched);
    co_queue_append(sched->co_queue, sched->co_curr);
    unlock_obj(sched);

    schedule_yield_coroutine(sched, sched->co_curr, CO_WAITING);
}

void sched_yield_coroutine(sched_t sched)
{
    lock_obj(sched);
    co_queue_append(sched->co_ready_queue, sched->co_curr);
    unlock_obj(sched);
    
    schedule_yield_coroutine(sched, sched->co_curr, CO_WAITING);
}

void sched_await_coroutine(sched_t sched)
{
    schedule_suspend_coroutine(sched);
}

void sched_sched(sched_t sched, struct coroutine *co)
{
    assert(co->status == CO_RUNNABLE); 

    make_co_context(sched, co); 
    co->sched = (void *)sched;
    
    // to avoid deadlock, we must keep locks sequence: sched->lock first, then queue->lock.
    pthread_mutex_lock(&sched->lock);     
    if(sched->co_nums == 0)
        pthread_cond_signal(&sched->cond);
    co_queue_append(sched->co_ready_queue, co);
    sched->co_nums++;
    pthread_mutex_unlock(&sched->lock);

    return;
}

void* sched_co_resume(void *args)
{
    struct coroutine *co = (struct coroutine *)args;
    sched_t sched = (sched_t)co->sched;
    
    list_del(&co->list);
    co->status = CO_RUNNABLE;
    co_queue_append(sched->co_ready_queue, co);

    return NULL;
}

static void* co_after_delay(void* args)
{
    struct coroutine *co = (struct coroutine *)args;
    sched_co_resume(co);

    return NULL;
}

void sched_delay(sched_t sched, long delay_ms)
{
    struct coroutine *co = sched->co_curr;
    co_timer_t ct = co_timer_after(delay_ms, co_after_delay, (void *)co);
    heap_push(sched->co_timer_heap, (void *)ct);

    sched_suspend_coroutine(sched);
}

int32_t sched_coroutine_num(sched_t sched)
{
    return sched->co_nums;
}

void sched_wakeup(sched_t sched)
{
    lock_obj(sched);
    sched->status = SCHED_STOPPED;
    pthread_cond_signal(&sched->cond);
    unlock_obj(sched);
}