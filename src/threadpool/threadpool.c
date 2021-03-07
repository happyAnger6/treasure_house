#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>

#include <sys/sysinfo.h>

#include "types.h"
#include "list.h"
#include "threadpool.h"

static void init_workqueue(struct workqueue *wq)
{
    INIT_LIST_HEAD(&wq->list);
    pthread_mutex_init(&wq->lock, NULL);
    pthread_cond_init(&wq->signal, NULL);
    wq->num = 0;
    wq->status = STATUS_RUNNING;
}

static struct work *workqueue_get(struct workqueue *wq)
{
    struct work *work = NULL, *tmp = NULL;

    pthread_mutex_lock(&wq->lock);
    while (wq->num == 0) {
        if (wq->status != STATUS_RUNNING)
            goto return_work;
        
        pthread_cond_wait(&wq->signal, &wq->lock);
    }
        
    list_for_each_entry_safe(work, tmp, &wq->list, list) {
        list_del(&work->list);
        wq->num -= 1;
        goto return_work;
    }

return_work:
    pthread_mutex_unlock(&wq->lock);
    return work;
}

static int workqueue_put(struct workqueue *wq, struct work *work)
{
    pthread_mutex_lock(&wq->lock);
    
    list_add_tail(&work->list, &wq->list);
    if (wq->num == 0)
        pthread_cond_signal(&wq->signal);
    
    wq->num += 1;
    pthread_mutex_unlock(&wq->lock);
    
    return 0;
}

static void workqueue_stop(struct workqueue *wq)
{
    pthread_mutex_lock(&wq->lock);
    wq->status = STATUS_STOPPING;
    pthread_cond_broadcast(&wq->signal);
    pthread_mutex_unlock(&wq->lock);
}

static void* do_work(struct work *work)
{
    struct task *task = NULL;
    void *ret = NULL;
    
    task = work->task;
    ret = task->do_task(task->args);

    if (task->clean_task)
        task->clean_task(task, ret);
    free(work);
    return ret;
}

static void* worker_fn(void *data)
{
    struct threadpool *pool = (struct threadpool *)data;
    struct workqueue *wq = pool->queue;
    struct work *work = NULL;
    
    while ((work = workqueue_get(wq)) != NULL)
        do_work(work);

    pthread_mutex_lock(&pool->lock);
    pool->threads_num -= 1;
    if (pool->threads_num == 0) {
        pool->status = STATUS_STOPPED;
        pthread_cond_signal(&pool->shutdown); 
    }
    
    pthread_mutex_unlock(&pool->lock);

    return NULL;
}

static int get_cpu_num()
{
    return get_nprocs();
}

struct threadpool *threadpool_create(int max_threads)
{
    struct threadpool *pool = NULL;(struct threadpool *)malloc(sizeof(struct threadpool));
    struct workqueue *queue = NULL;
    int thread_num;

    thread_num = MIN(get_cpu_num() * 3, max_threads);
    pool = (struct threadpool *)malloc(sizeof(struct threadpool) + sizeof(pthread_t) * thread_num);
    if (pool == NULL)
        return NULL;
    
    queue = (struct workqueue *)malloc(sizeof(struct workqueue));
    if (queue == NULL)
        goto free_pool;

    init_workqueue(queue);
    pool->queue = queue;
    pool->threads_num = 0;
    pool->max_threads = thread_num;
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->shutdown, NULL);    
    pool->status = STATUS_RUNNING;
    return pool;
    
free_pool:
    free(pool);
    return pool;
}

/*caller must hold pool->lock*/
static int create_new_thread(struct threadpool *pool)
{
    if (pthread_create(&pool->threads[pool->threads_num], NULL,
            worker_fn, (void *)pool) != 0)
        return -1;
        
    pool->threads_num += 1;
    return 0;
}

int threadpool_submit(struct threadpool *poolp, struct task *taskp)
{
    if (poolp == NULL)
        return -1;

    pthread_mutex_lock(&poolp->lock);
    if (poolp->status != STATUS_RUNNING) {
        goto failed;
    }

    if (poolp->threads_num < poolp->max_threads) {
        if (create_new_thread(poolp) < 0) /*TODO: should avoid create new thread when have an idle thread.*/
            goto failed;
    }        
    
    struct work *work = (struct work *)malloc(sizeof(struct work));
    work->task = taskp;
    (void)workqueue_put(poolp->queue, work); 

    pthread_mutex_unlock(&poolp->lock);
    return 0;

failed:
    pthread_mutex_unlock(&poolp->lock);
    return -1;
}

int threadpool_shutdown(struct threadpool *poolp, enum shutdown_type type, void *args)
{
    if (poolp == NULL)
        return -1;

    pthread_mutex_lock(&poolp->lock);

    poolp->status = STATUS_STOPPING;
    workqueue_stop(poolp->queue);

    if (poolp->threads_num == 0) {
        poolp->status = STATUS_STOPPED;
        pthread_cond_signal(&poolp->shutdown);
    }
    
    pthread_mutex_unlock(&poolp->lock);
    return 0;
}

int threadpool_wait(struct threadpool *poolp)
{
    pthread_mutex_lock(&poolp->lock);
    
    while (poolp->status != STATUS_STOPPED)
        pthread_cond_wait(&poolp->shutdown, &poolp->lock);

    pthread_mutex_unlock(&poolp->lock);
    return 0;
}

