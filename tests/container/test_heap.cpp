#include <gtest/gtest.h>
#include "heap.h"

static inline int voidp_to_int(void* ptr)
{
    return static_cast<int>(reinterpret_cast<intptr_t>(ptr));
}

int cmp_int(void *pa, void *pb)
{
    int a = voidp_to_int(pa);
    int b = voidp_to_int(pb);

    return a - b;
}

TEST(TEST_HEAP, test_heap_empty)
{
    heap_t ht = heap_create(cmp_int);
    
    EXPECT_EQ(heap_pop(ht), nullptr);
    EXPECT_EQ(heap_pop(ht), nullptr);
}

TEST(TEST_HEAP, test_heap_pop)
{
    heap_t ht = heap_create(cmp_int);
    heap_push(ht, (void *)10);
    heap_push(ht, (void *)20);
    heap_push(ht, (void *)6);
    heap_push(ht, (void *)5);
    heap_push(ht, (void *)2);

    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 2);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 5);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 6);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 10);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 20);
    EXPECT_EQ(heap_pop(ht), nullptr);
}

TEST(TEST_HEAP, test_heap_order_push)
{
    heap_t ht = heap_create(cmp_int);
    heap_push(ht, (void *)2);
    heap_push(ht, (void *)4);
    heap_push(ht, (void *)6);
    heap_push(ht, (void *)8);
    heap_push(ht, (void *)10);

    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 2);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 4);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 6);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 8);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 10);
    EXPECT_EQ(heap_pop(ht), nullptr);
}

TEST(TEST_HEAP, test_heap_reverse_order_push)
{
    heap_t ht = heap_create(cmp_int);
    heap_push(ht, (void *)10);
    heap_push(ht, (void *)8);
    heap_push(ht, (void *)6);
    heap_push(ht, (void *)4);
    heap_push(ht, (void *)2);

    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 2);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 4);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 6);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 8);
    EXPECT_EQ(voidp_to_int(heap_pop(ht)), 10);
    EXPECT_EQ(heap_pop(ht), nullptr);
}