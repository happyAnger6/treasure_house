#include "gtest/gtest.h"

#include "threadpool.h"

TEST(threadpool, create_success) 
{
    struct threadpool* thpl = threadpool_create(5);
    EXPECT_NE(thpl, nullptr);
    EXPECT_EQ(thpl->threads_num, 0);
    EXPECT_EQ(thpl->status, STATUS_RUNNING);

    threadpool_shutdown(thpl, WAIT_ALL_DONE, NULL);
    threadpool_wait(thpl);
}

void* task_fn(void *args)
{
    int *pval = (int *)args;
    *pval += 10;

    return NULL;
}

TEST(threadpool, submit_one_work) 
{
    int val = 10;
    struct threadpool* thpl = threadpool_create(5);
    struct task one_task = {
        .do_task = task_fn,
        .args = (void *)&val
    };
    
    EXPECT_NE(thpl, nullptr);
    EXPECT_EQ(thpl->threads_num, 0);
    EXPECT_EQ(thpl->status, STATUS_RUNNING);

    threadpool_submit(thpl, &one_task);
    threadpool_shutdown(thpl, WAIT_ALL_DONE, NULL);
    threadpool_wait(thpl);

    EXPECT_EQ(val, 20);
}

TEST(threadpool, submit_three_work) 
{
    int val1 = 10, val2 = 20, val3 = 100;
    struct threadpool* thpl = threadpool_create(5);
    struct task one_task = {
        .do_task = task_fn,
        .args = (void *)&val1
    };

    struct task two_task = {
        .do_task = task_fn,
        .args = (void *)&val2
    };

    struct task three_task = {
        .do_task = task_fn,
        .args = (void *)&val3
    };
    
    EXPECT_NE(thpl, nullptr);
    EXPECT_EQ(thpl->threads_num, 0);
    EXPECT_EQ(thpl->status, STATUS_RUNNING);

    threadpool_submit(thpl, &one_task);
    threadpool_submit(thpl, &two_task);
    threadpool_submit(thpl, &three_task);
    threadpool_shutdown(thpl, WAIT_ALL_DONE, NULL);
    threadpool_wait(thpl);

    EXPECT_EQ(val1, 20);
    EXPECT_EQ(val2, 30);
    EXPECT_EQ(val3, 110);
}