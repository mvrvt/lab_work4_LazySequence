#include <gtest/gtest.h>
#include "../src/LazySequence.hpp"
#include "../src/MyUtils.hpp"

using namespace my_utils;

// Строго функции вместо лямбд!
int BaseRule(int index) { return index + 1; } // 1, 2, 3, 4...
int MapRule(const int& x) { return x * 2; }   // 2, 4, 6, 8...
int Seq2Rule(int index) { return 100 + index; } // 100, 101, 102...

LazySequence<int>* CreateInfiniteSeq() {
    return new LazySequence<int>(BaseRule, Ordinal(1, 0), 10);
}

TEST(LazySequenceTest, GetElements) {
    auto* seq = CreateInfiniteSeq();
    EXPECT_EQ(seq->GetByOrdinal(Ordinal(0, 0)), 1);
    EXPECT_EQ(seq->GetByOrdinal(Ordinal(0, 1)), 2);
    EXPECT_EQ(seq->GetByOrdinal(Ordinal(0, 9)), 10);
    delete seq;
}

TEST(LazySequenceTest, MapOperation) {
    auto* seq = CreateInfiniteSeq();
    auto* mapped = seq->Map<int>(MapRule);
    
    EXPECT_EQ(mapped->GetByOrdinal(Ordinal(0, 0)), 2);
    EXPECT_EQ(mapped->GetByOrdinal(Ordinal(0, 1)), 4);
    EXPECT_EQ(mapped->GetByOrdinal(Ordinal(0, 2)), 6);

    delete mapped;
    delete seq;
}

TEST(LazySequenceTest, SkipOperation) {
    auto* seq = CreateInfiniteSeq();
    auto* skipped = seq->Skip(5);
    
    EXPECT_EQ(skipped->GetByOrdinal(Ordinal(0, 0)), 6);
    EXPECT_EQ(skipped->GetByOrdinal(Ordinal(0, 1)), 7);

    delete skipped;
    delete seq;
}

TEST(LazySequenceTest, OrdinalConcatInfinities) {
    LazySequence<int>* seq1 = CreateInfiniteSeq();
    LazySequence<int>* seq2 = new LazySequence<int>(Seq2Rule, Ordinal(1, 0), 10);

    LazySequence<int>* concat_seq = seq1->ConcatSequence(seq2);

    // Мощность должна стать w0 * 2
    EXPECT_EQ(concat_seq->GetOrdinalCardinality().omega0, 2);

    // Берем элемент из ПЕРВОЙ бесконечности
    EXPECT_EQ(concat_seq->GetByOrdinal(Ordinal(0, 5)), 6);

    // Берем элемент из ВТОРОЙ бесконечности (Магия на доске препода!)
    EXPECT_EQ(concat_seq->GetByOrdinal(Ordinal(1, 5)), 105);

    delete concat_seq;
    delete seq2;
    delete seq1;
}

TEST(SlidingCacheTest, CacheHitsAndShifts) {
    SlidingCache<int> cache(3);
    cache.Push(10, 0);
    cache.Push(20, 1);
    cache.Push(30, 2);
    
    EXPECT_TRUE(cache.Contains(0));
    EXPECT_EQ(cache.Get(1), 20);

    // Переполнение
    cache.Push(40, 3);
    EXPECT_FALSE(cache.Contains(0)); // 0 вытолкнулся
    EXPECT_TRUE(cache.Contains(3));
    EXPECT_EQ(cache.Get(3), 40);
}
