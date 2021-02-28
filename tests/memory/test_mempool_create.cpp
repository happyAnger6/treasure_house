#include "gtest/gtest.h"

#include "mempool.h"

TEST(create, small_size_3) {
    struct mempool* mp = mempool_create("small_pool", 3, 4, NULL, NULL, 0);
    EXPECT_NE(mp, nullptr);
    EXPECT_EQ(mp->size, 4);
    EXPECT_EQ(mp->obj_size, 3);
    EXPECT_EQ(mp->align, 4);

    mempool_destory(mp);
}