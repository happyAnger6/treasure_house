#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "gtest/gtest.h"

#include "processor.h"
#include "coroutine.h"
#include "asyncio.h"
#include "runtime/chan.h"

static int co_seq[200] = {0};
static g_seq = 0;

static void co_producer(void *args)
{
   chan_t c = (chan_t)args;
   for (int i = 0; i < 100; i++)
   {
       co_seq[g_seq++] = i;
       printf("co write[%d]\r\n", i);
       channel_write(c, (void *)i);
   }
}

static void co_consumer(void *args)
{
   chan_t c = (chan_t)args;
   int w;
   for (int i = 0; i < 100; i++)
   {
       channel_read(c, (void *)&w);
       co_seq[g_seq++] = i;
       printf("co read[%d]\r\n", w);
   }

}

TEST(channel_test, no_buffer_channel_ok) 
{
    chan_t chan = channel_create(sizeof(int));

    coroutine_init();
    coroutine_create(co_producer, (void*)chan);
    coroutine_create(co_consumer, (void*)chan);

    coroutine_loop();

    for(int i = 0; i < 200; i++)
        EXPECT_EQ(co_seq[i], i/2);
}