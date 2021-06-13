#include <stddef.h>
#include <time.h>

#include "co_time.h"

static long time_now()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_MONOTONIC, &now);
    long now_ms = s_to_ms(now.tv_sec) + ns_to_ms(now.tv_nsec);
    return now_ms;
}

co_timer_t co_timer_after(long after_ms, co_timer_callback callback, void *args)
{
    _co_timer_t *ct = (_co_timer_t *)malloc(sizeof(_co_timer_t));
    ct->when = time_now() + after_ms;
    ct->callback = callback;
    ct->args = args;
    return (co_timer_t)ct;
}

void co_timer_destory(co_timer_t co_timer)
{
    if (co_timer != NULL)
        free(co_timer);
}

void* co_timer_run(co_timer_t co_timer)
{
    _co_timer_t *_co_timer;
    if(_co_timer == NULL || _co_timer->callback == NULL)
        return NULL;

    return _co_timer->callback(_co_timer->args);
}

int co_timer_cmp(void* timer1, void *timer2)
{
    co_timer_t *ct1 = (co_timer_t *)timer1;
    co_timer_t *ct2 = (co_timer_t *)timer2;

    long when1 = ct1->when;
    long when2 = ct2->when;
    if(when1 == when2)
        return 0;

    return when1 > when2;
}

long co_timer_now()
{
    return time_now();
}