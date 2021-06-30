#ifndef _FUTURE_H
#define _FUTURE_H

#ifdef __cplusplus
extern "C"{
#endif

/* 
**
**
** example code:
** future_t f = async_create_connection(host, port);
   coroutine_await_future(f);
   connecttion c = f.get();
   return c;
*/
typedef enum {
    PENDING = 0,
    CANCELLED,
    FINISHED
}future_status_e;

typedef void* future_t;
typedef void* (*future_done_callback)(void *args);

/* Return future_t
** future_t must be called with future_await, after future await,
**   future_t will be freed.
*/
extern future_t future_create();
extern void future_destory(future_t f);

extern int future_add_done_callback(future_t future, future_done_callback cb, void *args);

extern int future_set_result(future_t future, void *result);
extern void* future_get_result(future_t future);

extern int future_get_errno(future_t future);

ASYNC extern void* future_await(future_t future);
extern int future_done(future_t future);

#ifdef __cplusplus
}
#endif

#endif