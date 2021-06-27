#ifndef _ASYNCIO_SERVER_H
#define _ASYNCIO_SERVER_H

#ifdef __cplusplus
extern "C"{
#endif

#include "streams.h"
#include "event_loop.h"

typedef void (*client_cb)(transport_t *trans, void *args);
typedef struct server *server_t;

extern server_t server_create(const char *host, int port, client_cb cb, void *args);
extern void server_loop(server_t server);

#ifdef __cplusplus
}
#endif

#endif