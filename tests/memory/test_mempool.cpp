#include "gtest/gtest.h"

#include "mempool.h"

TEST(create, size_3) 
{
    struct mempool* mp = mempool_create("size_3", 3, 4, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 3);
    EXPECT_EQ(mp->size, sizeof(void *));
    EXPECT_EQ(mp->align, sizeof(void *));
    EXPECT_EQ(mp->order, 0);

    mempool_destory(mp);
}

TEST(create, size_100) 
{
    struct mempool* mp = mempool_create("size_100", 100, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 100);
    EXPECT_EQ(mp->size, 104);
    EXPECT_EQ(mp->align, 8);

    mempool_destory(mp);
}

TEST(create, size_1000) 
{
    struct mempool* mp = mempool_create("size_1000", 1000, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 1000);
    EXPECT_EQ(mp->size, 1000);
    EXPECT_EQ(mp->align, 8);
    EXPECT_EQ(mp->order, 1);

    mempool_destory(mp);
}

TEST(alloc, obj_size5) 
{
    struct mempool* mp = mempool_create("size_5", 5, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);

    void *obj = mempool_alloc(mp, 0);
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(((intptr_t)obj)&(8 - 1), 0);

    mempool_destory(mp);
}