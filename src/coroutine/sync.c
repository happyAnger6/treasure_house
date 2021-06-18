#include <stdio.h>
#include <stdlib.h>
#include "sync.h"

#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int val;
}_wait_group_t;

wait_group_t wait_group_create()
{
    _wait_group_t *wg = (_wait_group_t *)malloc(sizeof(_wait_group_t));
    pthread_mutex_init(&wg->lock, NULL);
    pthread_cond_init(&wg->cond, NULL);
    wg->val = 0;
    return (wait_group_t)wg;
}

void wait_group_add(wait_group_t wg, int val)
{
    _wait_group_t *wg = (_wait_group_t *)wg;
    pthread_mutex_lock(&wg->lock);
    wg->val += val;
    pthread_mutex_unlock(&wg->lock);
}

void wait_group_done(wait_group_t wg)
{
    _wait_group_t *wg = (_wait_group_t *)wg;
    pthread_mutex_lock(&wg->lock);
    wg->val--;
    if (wg->val == 0)
        pthread_cond_signal(&wg->cond);
    pthread_mutex_unlock(&wg->lock);
}

void wait_group_wait(wait_group_t wg)
{
    _wait_group_t *wg = (_wait_group_t *)wg;
    pthread_mutex_lock(&wg->lock);
    while (wg->val != 0)
        pthread_cond_wait(&wg->cond, &wg->lock);
    pthread_mutex_unlock(&wg->lock);
    return 0;
}