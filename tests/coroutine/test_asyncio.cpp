#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "gtest/gtest.h"

#include "processor.h"
#include "coroutine.h"
#include "asyncio.h"

static void co_connect(void *args)
{
    
}

static void co_noblock(void *args)
{

}

TEST(asyncio_tcp_connect, connect_async_ok) 
{
    processors_set_maxprocs(1);

    coroutine_init();
    coroutine_create(co_connect, (void*)&a);
    coroutine_create(co_noblock, (void*)&b);

    coroutine_loop();

    EXPECT_EQ(g_run_sums, 6);
}