#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "gtest/gtest.h"

#include "coroutine.h"

static int g_pos = 0;
static int g_run_sums = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

void co(void *args)
{
    int num = *((int *)args);

    pthread_mutex_lock(&g_lock);
    g_run_sums += num;
    pthread_mutex_unlock(&g_lock);

    printf("coroutine:%d begin...\r\n", num);
    coroutine_yield();
    printf("coroutine:%d ended...\r\n", num);
}

TEST(coroutine_create, three_cos_take_turns) 
{
    int a = 1, b = 2, c = 3;
    coroutine_init();
    coroutine_create(co, (void*)&a);
    coroutine_create(co, (void*)&b);
    coroutine_create(co, (void*)&c);

    coroutine_loop();

    EXPECT_EQ(g_run_sums, 6);
}
