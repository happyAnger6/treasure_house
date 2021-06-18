#include <gtest/gtest.h>
#include "heap.h"

int cmp_int(void *pa, void *pb)
{
    int a = (int)pa;
    int b = (int)pb;

    if (a == b)
        return 0;

    return a > b;
}

TEST(TEST_HEAP, test_heap_pop)
{
    heap_t ht = heap_create();
    heap_push(ht, (void *)10);
    heap_push(ht, (void *)20);
    heap_push(ht, (void *)6);
    heap_push(ht, (void *)5);
    heap_push(ht, (void *)2);

    EXPECT_EQ((int)heap_push(ht), 2);
    EXPECT_EQ((int)heap_push(ht), 5);
    EXPECT_EQ((int)heap_push(ht), 6);
    EXPECT_EQ((int)heap_push(ht), 10);
    EXPECT_EQ((int)heap_push(ht), 20);
    EXPECT_EQ(heap_push(ht), NULL);
}
