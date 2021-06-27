#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"

struct server {
    int fd;
    int port;
    char *host;
    client_cb cli_cb;
    void *cli_args;
    transport_t *transport;
    event_loop_t ev_loop;
    fd_map_t fd_map;
};

server_t server_create(const char *host, int port, client_cb cb, void *args)
{
    struct server *st = (struct server *)malloc(sizeof(struct server));
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return NULL;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, (void *)addr.sin_addr) != 1)
        goto err_exit;

    if (bind(fd, (const struct sockaddr*)addr, sizeof(addr)) != 0)
        goto err_exit;

    if (listen(fd, SOMAXCONN) != 0)
        goto err_exit;

    st->transport = transport_create(fd);
    st->fd_map = fd_map_create();
    st->cli_cb = cb;
    st->cli_args = args;
    st->fd = fd;

    return st;

err_exit:
    close(fd);
    return NULL;
}

static void accept_cb(void *args, future_t f)
{
    struct server *svr = (struct server *)args;
    int fd = accept(server->fd, NULL, NULL);
    if (fd < 0) 
    {
        if (errno == EINTR) 
        {}
        else if (errno != EAGAIN && errno != EWOULDBLOCK) 
        {
            future_set_result(f, NULL);
            future_set_errno(f, errno);
            processors_remove_server(srv);
        }
    }
    else
    {
        connection_t c = connection_create(fd);
        server_add_connection(srv, c);
    }
}

int server_forever(server_t server) 
{
    int fd = server->fd;

    if (listen(fd, SOMAXCONN) != 0)
        goto err_exit;

    future_t f = processors_create_co_future();
    processors_add_server(server, accept_cb, (void *)server, f);

    return AWAIT future_awit(f);
}