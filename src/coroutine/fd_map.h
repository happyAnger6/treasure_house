#ifndef _FD_MAP_H
#define _FD_MAP_H

#define MAX_FDS 65535

typedef struct {
    void *fd_map[MAX_FDS];
} fd_map_t;

fd_map_t* fd_map_create();
int fd_map_add_fd(fd_map_t* fdt, int fd, void *value);
int fd_map_remove_fd(fd_map_t* fdt, int fd);
int fd_map_destory(fd_map_t*  fdt);

#endif