#include "gtest/gtest.h"

#include "mempool.h"

TEST(create, size_3) 
{
    struct mempool* mp = mempool_create("create_size_3", 3, 4, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 3);
    EXPECT_EQ(mp->size, sizeof(void *));
    EXPECT_EQ(mp->align, sizeof(void *));
    EXPECT_EQ(mp->order, 0);

    mempool_destory(mp);
}


TEST(create, size_12) 
{
    struct mempool* mp = mempool_create("create_size_12", 12, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 12);
    EXPECT_EQ(mp->size, 16);
    EXPECT_EQ(mp->align, 8);

    mempool_destory(mp);
}

TEST(create, size_100) 
{
    struct mempool* mp = mempool_create("create_size_100", 100, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 100);
    EXPECT_EQ(mp->size, 104);
    EXPECT_EQ(mp->align, 8);

    mempool_destory(mp);
}

TEST(create, size_1000) 
{
    struct mempool* mp = mempool_create("create_size_1000", 1000, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->obj_size, 1000);
    EXPECT_EQ(mp->size, 1000);
    EXPECT_EQ(mp->align, 8);
    EXPECT_EQ(mp->order, 1);

    mempool_destory(mp);
}

TEST(alloc, obj_size5_1) 
{
    struct mempool* mp = mempool_create("alloc_size_5", 5, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);

    void *obj = mempool_alloc(mp, 0);
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(((intptr_t)obj)&(8 - 1), 0);

    EXPECT_NE(mp->slabs.cur, nullptr);
    EXPECT_EQ(mp->slabs.slab_nums, 1);
    EXPECT_EQ(mp->slabs.used_objs, 1);
    EXPECT_EQ(mp->slabs.total_objs, mp->num_per_slab);
    EXPECT_EQ(mp->slabs.total_objs, mp->slabs.free_objs + mp->slabs.used_objs);
    mempool_destory(mp);
}

TEST(alloc, obj_size5_10) 
{
    struct mempool* mp = mempool_create("alloc_size_5_10", 5, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);

    void *obj = NULL;
    for (size_t i = 0; i < 10; i++)
    {   
        obj = mempool_alloc(mp, 0);
        EXPECT_NE(obj, nullptr);
        EXPECT_EQ(((intptr_t)obj)&(8 - 1), 0);
    }

    EXPECT_NE(mp->slabs.cur, nullptr);
    EXPECT_EQ(mp->slabs.slab_nums, 1);
    EXPECT_EQ(mp->slabs.used_objs, 10);
    EXPECT_EQ(mp->slabs.total_objs, mp->num_per_slab);
    EXPECT_EQ(mp->slabs.total_objs, mp->slabs.free_objs + mp->slabs.used_objs);
    mempool_destory(mp);
}

TEST(free, obj_size12)
{
    struct mempool* mp = mempool_create("free_size_12", 5, 8, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);

    void *obj = mempool_alloc(mp, 0);
    EXPECT_NE(obj, nullptr);

    EXPECT_NE(mp->slabs.cur, nullptr);
    EXPECT_EQ(mp->slabs.slab_nums, 1);
    EXPECT_EQ(mp->slabs.used_objs, 1);
    EXPECT_EQ(mp->slabs.total_objs, mp->num_per_slab);
    EXPECT_EQ(mp->slabs.total_objs, mp->slabs.free_objs + mp->slabs.used_objs);

    mempool_free(mp, obj);
    EXPECT_EQ(mp->slabs.used_objs, 0);
    mempool_destory(mp);
}
