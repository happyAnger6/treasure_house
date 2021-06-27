#ifndef _ASYNCIO_STREAMS_H
#define _ASYNCIO_STREAMS_H

#ifdef __cplusplus
extern "C"{
#endif

#include "asyncio.h"
#include "future.h"

static inline read_error(int ret)
{
    return ret == 0 || (errno != EAGAIN && errno != EWOULDBLOCK);
}

typedef struct connection *connection_t;

typedef struct {
    future_t data_waiter;
    transport_t *transport;
} stream_reader_t;

ASYNC int asyncio_open_connection(char *host, int port);

void stream_reader_feed_data(stream_reader_t *sr, void *data, size_t data_len);

ASYNC ssize_t stream_reader_read(stream_reader_t *sr, void *buf, size_t cnt);

ASYNC ssize_t connection_read(connection_t conn, void *buf, size_t count);

connection_t connection_create(int fd);
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__ASYNCIO_STREAMS_H