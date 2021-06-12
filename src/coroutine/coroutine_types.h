#ifndef _COROUTINE_TYPES_H
#define _COROUTINE_TYPES_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "list.h"

typedef struct list_head queue;

static inline void error_exit(char *msg)
{
    perror(msg);
    exit(1);
}

#ifdef __cplusplus
}
#endif

#endif