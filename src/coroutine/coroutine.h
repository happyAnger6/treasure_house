#ifndef _COROUTINE_H
#define _COROUTINE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <ucontext.h>

#include "list.h"

#define CO_STACK_SIZE 16*1024

typedef void (*coroutine_func)(void*);

typedef void* coroutine_handle;

extern int coroutine_init();
extern coroutine_handle coroutine_create(coroutine_func co_fn, void *args);
extern int coroutine_yield();
extern int coroutine_loop();

#endif

#ifdef __cplusplus
}
#endif