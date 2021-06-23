#ifndef _COROUTINE_ASYNCIO_H
#define _COROUTINE_ASYNCIO_H 

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#define ASYNC

extern void asyncio_sleep(long delay_ms);

#ifdef __cplusplus
}
#endif

#endif