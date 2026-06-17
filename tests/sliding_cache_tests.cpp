#include <gtest/gtest.h>
#include "../src/SlidingCache.hpp"

TEST(SlidingCacheTest, Logic) {
    SlidingCache<int> cache(3);
    cache.Push(10, 0);
    cache.Push(20, 1);
    cache.Push(30, 2);

    EXPECT_TRUE(cache.Contains(1));
    EXPECT_EQ(cache.Get(1), 20);

    cache.Push(40, 3); // Выталкивание первого элемента из скользящего буфера
    EXPECT_FALSE(cache.Contains(0));
    EXPECT_TRUE(cache.Contains(3));
}
