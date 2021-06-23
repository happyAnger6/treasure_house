#include <stdio.h>
#include "future.h"
#include "asyncio.h"

typedef struct {
    int errno;
    future_status_e status;
    void *result;
    future_done_callback done_callback;
    void *done_args;
}_future_t;

future_t future_create()
{
    _future_t *f = (_future_t *)malloc(sizeof(_future_t));

    f->errno = 0;
    f->status = PENDING;
    f->done_callback = NULL;
    f->done_args = NULL;
    f->result = NULL;

    return (future_t)f;
}

void future_destory(future_t future)
{
    _future_t *f = (_future_t *)future;
    free(f);
}

int future_add_done_callback(future_t future, future_done_callback cb, void *args)
{
    _future_t *f = (_future_t *)future;
    f->done_callback = cb;
    f->done_args = args;

    return 0;
}

int future_set_result(future_t future, void *result)
{
    _future_t *f = (_future_t *)future;
    f->result = result;

    if (f->done_callback)
        f->done_callback(f->done_args);

    f->status = FINISHED;
    return 0;
}

int future_done(future_t future)
{
    return -1;
}

ASYNC void* future_await(future_t future)
{
    _future_t *f = (_future_t *)future;
    if (f->result != FINISHED)
        AWAIT processors_await();

    void* result = future_get_result(future);
    future_destory(future);
    return result;
}
