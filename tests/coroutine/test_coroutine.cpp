#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "gtest/gtest.h"

#include "coroutine.h"
#include "asyncio.h"

static int g_pos = 0;
static int g_run_sums = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void co(void *args)
{
    int num = *((int *)args);

    pthread_mutex_lock(&g_lock);
    g_run_sums += num;
    pthread_mutex_unlock(&g_lock);

    printf("coroutine:%d begin...\r\n", num);
    coroutine_yield();
    printf("coroutine:%d ended...\r\n", num);
}

TEST(coroutine_create, three_cos_run_success) 
{
    int a = 1, b = 2, c = 3;
    coroutine_init();
    coroutine_create(co, (void*)&a);
    coroutine_create(co, (void*)&b);
    coroutine_create(co, (void*)&c);

    coroutine_loop();

    EXPECT_EQ(g_run_sums, 6);
}

static int seq = 0;
static int co_seq[2] = {0};

static void co_sleep(void *args)
{
    printf("co sleep begin.\r\n");
    asyncio_sleep(100);
    co_seq[seq++] = 100;
    printf("co sleep end.\r\n");
}

static void co_nosleep(void *args)
{
    printf("co no sleep begin.\r\n");
    co_seq[seq++] = 200;
    printf("co sleep end.\r\n");
}

TEST(coroutine_run, co_sleep) 
{
    coroutine_init();
    coroutine_create(co_sleep, NULL);
    coroutine_create(co_nosleep, NULL);

    coroutine_loop();

    EXPECT_EQ(co_seq[0], 200);
    EXPECT_EQ(co_seq[0], 100);
}
