#include <pthread.h>

#include "gtest/gtest.h"

#include "threadpool.h"
#include "atm.h"

void* add_fn(void *arg)
{
    int *pval = (int *)arg;
    for(int i = 0; i < 100000; i++)
        atm_full_fetch_add(pval, 1);

    return NULL;
}

TEST(atm, add)
{
    int val = 0;
    struct threadpool* thpl = threadpool_create(5);
    EXPECT_NE(thpl, nullptr);
    
    struct task one_task = {
        .do_task = add_fn,
        .args = (void *)&val
    };

    struct task two_task = {
        .do_task = add_fn,
        .args = (void *)&val
    };

    struct task three_task = {
        .do_task = add_fn,
        .args = (void *)&val
    };
    
    threadpool_submit(thpl, &one_task);
    threadpool_submit(thpl, &two_task);
    threadpool_submit(thpl, &three_task);
    threadpool_shutdown(thpl, WAIT_ALL_DONE, NULL);
    threadpool_wait(thpl);

    
    EXPECT_EQ(val, 300000);
}
