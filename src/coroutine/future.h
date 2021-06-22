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


typdef void* future_t;

typedef struct {
    void *result;
    int status;
}_future_t;

extern int future_done(future_t f);

#ifdef __cplusplus
}
#endif

#endif