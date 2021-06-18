#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "event_loop.h"

#define MAX_EVENTS 16

typedef struct {
    int ep_fd;
    struct epoll_event events[MAX_EVENTS];
} event_loop_engine_t;

event_loop_t event_loop_create()
{
    event_loop_engine_t *ev_loop = (event_loop_engine_t *)malloc(sizeof(event_loop_engine_t));
    ev_loop->ep_fd = epoll_create1(EPOLL_CLOEXEC);

    return (event_loop_t)ev_loop;
}

extern int event_loop_run(event_loop_t ev_loop, long expired_ms)
{
    event_loop_engine_t *evt_loop = (event_loop_engine_t *)ev_loop;
    return epoll_pwait(evt_loop->ep_fd, evt_loop->events, MAX_EVENTS,
                (int)expired_ms, NULL);
}