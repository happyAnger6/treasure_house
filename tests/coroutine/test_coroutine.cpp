#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

#include "coroutine.h"

static int g_pos = 0;
static int g_run_seq_ary[6] = {0};

void co(void *args)
{
    int num = (int)args;
    g_run_seq_ary[g_pos++] = num;
    printf("coroutine:%d begin...\r\n", num);
    coroutine_yield();
    g_run_seq_ary[g_pos++] = num;
    printf("coroutine:%d ended...\r\n", num);
}

TEST(coroutine_create, three_cos_take_turns) 
{
    coroutine_init();
    coroutine_create(co, (void*)1);
    coroutine_create(co, (void*)2);
    coroutine_create(co, (void*)3);

    coroutine_loop();

    EXPECT_EQ(g_run_seq_ary[0], 1);
    EXPECT_EQ(g_run_seq_ary[1], 2);
    EXPECT_EQ(g_run_seq_ary[2], 3);
    EXPECT_EQ(g_run_seq_ary[3], 1);
    EXPECT_EQ(g_run_seq_ary[4], 2);
    EXPECT_EQ(g_run_seq_ary[5], 3);
}
