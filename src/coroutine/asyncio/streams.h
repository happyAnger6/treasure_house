#ifndef _ASYNCIO_STREAMS_H
#define _ASYNCIO_STREAMS_H

#ifdef __cplusplus
extern "C"{
#endif

#include "asyncio.h"

ASYNC void asyncio_open_connection(char *host, int port);

#ifdef __cplusplus
}
#endif

#endif