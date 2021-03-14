#include "gtest/gtest.h"

#include "threadpool.h"
#include "atm.h"
#include "mpscq.h"

struct data {
    int val;
    struct mpscq_node n;
};

void* push_node(void *arg)
{
    struct mpscq *queue = (struct mpscq *)arg;
    struct data *data = NULL;
    for (int i = 0; i < 10000; i++) {
        data = (struct data *)malloc(sizeof(struct data));
        data->val = 1;
        mpscq_push(queue, &data->n);
    }

    return NULL;
}

int g_total_val = 0;

void *pop_node(void *arg)
{
    struct mpscq *queue = (struct mpscq *)arg;
    struct data *data = NULL;
    struct mpscq_node *node;
    
    while(g_total_val < 20000) {
        node = NULL;
        node = mpscq_pop(queue);
        if (node) {
            data = container_of(node, struct data, n);
            g_total_val += data->val;
            free(data);
            data = NULL;
        }
    }

    return NULL;
}

TEST(mpscq, push_pop)
{
    int val = 0;
    struct threadpool* thpl = threadpool_create(5);
    EXPECT_NE(thpl, nullptr);
    struct mpscq queue;
    mpscq_create(&queue);

    struct task product1 = {
        .do_task = push_node,
        .args = (void *)&queue
    };

    struct task product2 = {
        .do_task = push_node,
        .args = (void *)&queue
    };

    struct task consumer = {
        .do_task = pop_node,
        .args = (void *)&queue
    };
    
    threadpool_submit(thpl, &product1);
    threadpool_submit(thpl, &product2);
    threadpool_submit(thpl, &consumer);
    threadpool_shutdown(thpl, WAIT_ALL_DONE, NULL);
    threadpool_wait(thpl);

    
    EXPECT_EQ(g_total_val, 20000);
}
