#ifndef _ASYNCIO_STREAM_READER_H
#define _ASYNCIO_STREAM_READER_H

#include "asyncio.h"
#include "future.h"

typedef struct {
    future_t data_waiter;
    void *transport;
    char *data_buf;
    size_t data_buf_len;
}stream_reader_t;

void stream_reader_feed_data(stream_reader_t *sr, void *data, size_t data_len);

ASYNC ssize_t stream_reader_read(stream_reader_t *sr, void *buf, size_t cnt);


#endif