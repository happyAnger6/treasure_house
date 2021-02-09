#include <gtest/gtest.h>
#include "list.h"

TEST(TEST_LIST, test_init)
{
    struct list_head lists;
    INIT_LIST_HEAD(&lists);
    ASSERT_TRUE(list_empty(&lists));
}
