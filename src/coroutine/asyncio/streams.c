#include <errno.h>

#include "processor.h"
#include "stream_reader.h"
#include "event_loop.h"
#include "events.h"

struct connect_args {
    future_t f;
    int sockfd;
    struct sockaddr_in *addr;
};

struct buffer {
    char *data;
    int size;
    int head;
    int tail;
    int free;
};

struct connection {
    int fd;
    struct sockaddr *addr;
    struct buffer *rbuf;
    struct buffer *wbuf;
    future_t rwaiter;
    future_t wwaiter;
}; 

static inline int read_nointerrupt(int fd, void *buf, int len)
{
    int ret = 0;
    do {
        ret = read(fd, buf, len);
    } while (ret < 0 && errno = EINTR);

    return ret;
}

static void* connection_read_data_nonblock(void *args)
{
    struct connection *conn = (struct connection *)args;
    future_t f = conn->rwaiter;
    char buf[4096];

    int ret = read_nointerrupt(conn->fd, buf, sizeof(buf));
    if (read_error(ret))
    {
        event_loop_remove_reader(conn->fd);
        set_future(f, NULL, errno);
    }
    else if (ret > 0)
    {
        buffer_add_data(conn->rbuf, buf, ret);
        event_loop_remove_reader(conn->fd);
        set_future(f, NULL, 0);
    }

    return NULL;
}

ASYNC int connection_read(struct connection *conn, void *buf, int len)
{
    int ret = buffer_get_data(conn->rbuf, buf, len);
    if (ret > 0)
        return ret;

    conn->rwaiter = processors_create_co_future();
    event_loop_add_reader(conn->fd, connection_read_data_nonblock, (void *)conn);

    AWAIT future_await(conn->rwaiter);
    return buffer_get_data(conn->rbuf, buf, len);
}

static inline int buffer_empty(struct buffer *buffer)
{
    return buffer->head == buffer->tail;
}

static inline int buffer_full(struct buffer *buffer)
{
    return (buffer->tail + 1) % buffer->size == buffer->head;
}

static struct buffer* buffer_create(int size)
{
    struct buffer* buf = (struct buffer *)malloc(sizeof(struct buffer));
    buffer->size = size;
    buffer->data = (char *)malloc(size);
    buffer->head = buffer->tail = 0;
    buffer->free = size - 1; //last not use to indicate full.
    return buf;
}

static int buffer_get_data(struct buffer* buf, char *data, int len)
{

}

static int buffer_add_data(struct buffer* buf, char *data, int len)
{

}

static inline set_future(future_t f, void *result, int errno)
{
    future_set_result(f, result);
    future_set_errno(f, errno);
}

static void* tcp_connect(void *args)
{
    struct connect_args *c_args = (struct connect_args *)args;
    int fd = c_args->sockfd;

    int ret = connect(fd, (struct sockaddr *)c_args->addr, 
                        sizeof(struct sockaddr_in));
    if (ret != 0 && errno != EINPROGRESS) {
        event_loop_remove_reader(fd);
        close(fd);
        set_future(c_args->f, (void *)-1, errno);
        free(c_args);
    }
    else if (ret == 0)
    {
        set_future(c_args->f, (void *)fd, 0);
        event_loop_remove_reader(fd);
        free(c_args);
    }

    return NULL;
}

static future_t async_tcp_connect(int sockfd, struct sockaddr_in *addr)
{
    future_t f = processors_create_co_future();
   
    struct connect_args *c_args = (struct connect_args *)malloc(sizeof(struct connect_args));
    c_args->f = f;
    c_args->sockfd = sockfd;
    c_args->addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    c_args->addr = *addr;

    event_loop_add_reader(sockfd, tcp_connect, (void *)c_args);
    
    return f;
}

static int make_sockaddr(char *host, int port, struct sockaddr_in *addr)
{
    addr.sin_family = AF_INT;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1)
        return -1;

    return 0;
}

connection_t connection_create(int fd)
{
    connection_t c = (connection_t)malloc(sizeof(struct connection));
    c->fd = fd;
    return c;
}

ASYNC connection_t asyncio_open_connection(char *host, int port)
{
    struct sockaddr_in addr;
    int ret = make_sockaddr(host, port, &addr) ;
    if (ret != 0) 
        return NULL;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return NULL;

    events_set_fd_nonblock(fd);
    ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == 0)
        goto ok;
    
    if (errno != EINPROGRESS)
    {
        close(sockfd);
        return NULL;
    }

    future_t f = async_tcp_connect(sockfd, &addr);
    AWAIT future_await(f);
ok:
    return create_connection(sockfd);
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
