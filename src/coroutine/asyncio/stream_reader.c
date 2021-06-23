#include "stream_reader.h"

void stream_reader_feed_data(stream_reader_t *sr, void *data, size_t data_len)
{

}

ASYNC ssize_t stream_reader_read(stream_reader_t *sr, void *buf, size_t cnt)
{

}


ASYNC static void wait_for_data(stream_reader_t *sr)
{
    future_t f = sr->data_waiter;
    coroutine_await(f);
}
