#ifndef _CO_TIME_H
#define _CO_TIME_H 

#ifdef __cplusplus
extern "C"{
#endif

static inline long ms_to_ns(long ms)
{
    return ms * 1000 * 1000;
}

static inline long ns_to_ms(long ns)
{
    return ns / 1000 / 1000;
}

static inline long s_to_ms(long s)
{
    return s * 1000;
}

typedef struct {
    long when;
    co_timer_callback callback;
    void *args;
} _co_timer_t;

typedef void* (*co_timer_callback)(void *args);

typedef _co_timer_t* co_timer_t;

extern co_timer_t co_timer_after(long after_ms, co_timer_callback callback, void *args);

extern void co_timer_destory(co_timer_t timer);

extern int co_timer_cmp(void* timer1, void *timer2);

/* Return current time in ms.*/
extern long co_timer_now();

void* co_timer_run(co_timer_t co_timer);

#ifdef __cplusplus
}
#endif

#endif