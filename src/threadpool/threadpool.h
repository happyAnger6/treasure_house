#ifndef _TH_THREADPOOL_H
#define _TH_THREADPOOL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <pthread.h>
#include "list.h"

enum work_status {
    STATUS_INIT = 0,
    STATUS_RUNNING,
    STATUS_STOPPING,
    STATUS_STOPPED
};

struct task {
    void* (*do_task)(void *args);
    void* (*clean_task)(struct task *task, void *ret); /*use this fn to cleanup task, caller impl*/
    void* args;
};

struct workqueue {
    pthread_mutex_t lock;
    pthread_cond_t signal;
    struct list_head list;
    int num;
    enum work_status status;
};

struct work {
    struct list_head list;
    struct task *task;
};

struct threadpool {
    int max_threads;
    int threads_num;
    pthread_mutex_t lock;
    pthread_cond_t shutdown;
    enum work_status status;
    struct workqueue *queue;
    pthread_t threads[0];
};

enum shutdown_type {
    WAIT_ALL_DONE = 0,
    NOT_WAIT
};
        
extern struct threadpool *threadpool_create(int max_threads);
extern int threadpool_submit(struct threadpool *poolp, struct task *taskp);
extern int threadpool_shutdown(struct threadpool *poolp, enum shutdown_type type, void *args);
extern int threadpool_wait(struct threadpool *poolp);

#ifdef __cplusplus
}
#endif

#endif
