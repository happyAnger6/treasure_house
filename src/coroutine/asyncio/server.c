#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"

server_t* server_create(const char *host, int port, client_cb cb, void *args)
{
    server_t *st = (server_t *)malloc(sizeof(server_t));
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

    processors_add_server(st);
    return st;

err_exit:
    close(fd);
    return NULL;
}