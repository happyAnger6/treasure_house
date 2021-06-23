#include <errno.h>

#include "processor.h"
#include "stream_reader.h"
#include "event_loop.h"
#include "events.h"

typedef struct {
    future_t f;
    int sockfd;
    struct sockaddr_in *addr;
} connect_args_t;

static void* _tcp_connect(void *args)
{
    connect_args_t *c_args = (connect_args_t *)args;
    future_t f = c_args->f;

    int ret = connect(c_args->sockfd, (struct sockaddr *)c_args->addr, 
                sizeof(struct sockaddr_in));
    if (ret != 0 && errno != EINPROGRESS) {
        event_loop_remove_reader(c_args->sockfd);
        free(c_args);
        return NULL;
    }
    
    if (ret == 0)
    {
        future_set_result(f, (void *)c_args->sockfd);
    }
    else
    {
        close(sockfd);
        future_set_result(f, NULL);
        future_set_errno(f, errno);
    }

    event_loop_remove_reader(c_args->sockfd);
    free(c_args);
    return NULL;
}

static connect_args_t* _make_connect_args(int sockfd, struct sockaddr_in *addr)
{
    future_t f = processors_create_co_future();
   
    connect_args_t *c_args = (connect_args_t *)malloc(sizeof(connect_args_t));
    c_args->f = f;
    c_args->sockfd = sockfd;
    c_args->addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    memcpy(c_args->addr, addr, sizeof(struct sockaddr_in));

    return c_args;
}

static future_t _async_tcp_connect(int sockfd, struct sockaddr_in *addr)
{
    connect_args_t *c_args = _make_connect_args(sockfd, addr);
    event_loop_add_reader(sockfd, _tcp_connect, (void *)c_args);
    
    return f;
}

ASYNC int asyncio_open_connection(char *host, int port)
{
    int sockfd = -1;
    struct sockaddr_in addr;
    future_t f;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;

    addr.sin_family = AF_INT;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1)
        return -1;

    events_set_fd_nonblock(sockfd);
    int ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == 0)
    {
        return sockfd;
    }
    else if (errno == EINPROGRESS)
    {
        f = _async_tcp_connect(sockfd, &addr);
    }
    else 
    {
        close(sockfd);
        return -1;
    }

    AWAIT (int)future_await(f);
}

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
