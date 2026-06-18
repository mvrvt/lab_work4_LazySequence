#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/SlidingCache.hpp"

// === Базовая логика ===
TEST(SlidingCacheTest, DefaultConstruction) {
    SlidingCache<int> cache(5);
    EXPECT_TRUE(cache.IsEmpty());
    EXPECT_EQ(cache.GetCount(), 0);
    EXPECT_EQ(cache.GetCapacity(), 5);
}

TEST(SlidingCacheTest, InvalidCapacityThrows) {
    EXPECT_THROW(SlidingCache<int>(0), std::invalid_argument);
    EXPECT_THROW(SlidingCache<int>(-5), std::invalid_argument);
}

TEST(SlidingCacheTest, PushSingleElement) {
    SlidingCache<int> cache(5);
    cache.Push(100, 0);
    EXPECT_FALSE(cache.IsEmpty());
    EXPECT_EQ(cache.GetCount(), 1);
    EXPECT_TRUE(cache.Contains(0));
    EXPECT_EQ(cache.Get(0), 100);
}

TEST(SlidingCacheTest, PushMultipleElementsNoOverflow) {
    SlidingCache<int> cache(5);
    cache.Push(10, 0);
    cache.Push(20, 1);
    cache.Push(30, 2);
    EXPECT_EQ(cache.GetCount(), 3);
    EXPECT_EQ(cache.Get(1), 20);
    EXPECT_EQ(cache.Get(2), 30);
}

// === Логика сдвига и сброса  ===
TEST(SlidingCacheTest, CacheOverflowShiftsWindow) {
    SlidingCache<int> cache(3);
    cache.Push(10, 0);
    cache.Push(20, 1);
    cache.Push(30, 2);
    cache.Push(40, 3); 
    
    EXPECT_EQ(cache.GetCount(), 3);
    EXPECT_FALSE(cache.Contains(0));
    EXPECT_TRUE(cache.Contains(1));
    EXPECT_TRUE(cache.Contains(3));
    EXPECT_EQ(cache.Get(3), 40);
}

TEST(SlidingCacheTest, MultipleOverflows) {
    SlidingCache<int> cache(2);
    cache.Push(1, 0);
    cache.Push(2, 1);
    cache.Push(3, 2);
    cache.Push(4, 3);
    cache.Push(5, 4);

    EXPECT_FALSE(cache.Contains(0));
    EXPECT_FALSE(cache.Contains(2));
    EXPECT_TRUE(cache.Contains(3));
    EXPECT_TRUE(cache.Contains(4));
    EXPECT_EQ(cache.Get(3), 4);
    EXPECT_EQ(cache.Get(4), 5);
}

TEST(SlidingCacheTest, PushDisconnectedIndexResetsCache) {
    SlidingCache<int> cache(5);
    cache.Push(10, 0);
    cache.Push(20, 1);
    
    // Прыгаем далеко вперед
    cache.Push(99, 100);
    
    EXPECT_EQ(cache.GetCount(), 1);
    EXPECT_FALSE(cache.Contains(0));
    EXPECT_FALSE(cache.Contains(1));
    EXPECT_TRUE(cache.Contains(100));
    EXPECT_EQ(cache.Get(100), 99);
}

TEST(SlidingCacheTest, ClearCache) {
    SlidingCache<int> cache(5);
    cache.Push(10, 0);
    cache.Clear();
    EXPECT_TRUE(cache.IsEmpty());
    EXPECT_FALSE(cache.Contains(0));
    EXPECT_EQ(cache.GetCount(), 0);
}

// === Проверка исключений ===
TEST(SlidingCacheTest, GetMissingElementThrows) {
    SlidingCache<int> cache(3);
    cache.Push(10, 0);
    EXPECT_THROW(cache.Get(1), std::out_of_range);
}

TEST(SlidingCacheTest, GetElementBeforeWindowThrows) {
    SlidingCache<int> cache(2);
    cache.Push(1, 0);
    cache.Push(2, 1);
    cache.Push(3, 2); 
    EXPECT_THROW(cache.Get(0), std::out_of_range);
}

TEST(SlidingCacheTest, GetElementAfterWindowThrows) {
    SlidingCache<int> cache(3);
    cache.Push(10, 0);
    EXPECT_THROW(cache.Get(5), std::out_of_range);
}

TEST(SlidingCacheTest, GetFromEmptyCacheThrows) {
    SlidingCache<int> cache(5);
    EXPECT_THROW(cache.Get(0), std::out_of_range);
}

TEST(SlidingCacheTest, ContainsOnEmptyCacheReturnsFalse) {
    SlidingCache<int> cache(5);
    EXPECT_FALSE(cache.Contains(0));
    EXPECT_FALSE(cache.Contains(100));
}

TEST(SlidingCacheTest, PushNegativeIndexResetsCorrectly) {
    SlidingCache<int> cache(3);
    cache.Push(42, -5);
    EXPECT_TRUE(cache.Contains(-5));
    EXPECT_EQ(cache.Get(-5), 42);
}
