#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/LazySequence.hpp"
#include "../src/MyUtils.hpp"

using namespace my_utils;

// === Функции генераторов ===
int BaseRule(int idx) { return idx + 1; }
int MultRule(const int& x) { return x * 10; }
int Seq2Rule(int idx) { return 100 + idx; }
int ThrowRule(int idx) { throw std::runtime_error("Error"); }

// === Тесты базового доступа ===
TEST(LazySequenceTest, FiniteSequenceLength) {
    LazySequence<int> seq(BaseRule, Ordinal(0, 5));
    EXPECT_EQ(seq.GetLength(), 5);
}

TEST(LazySequenceTest, InfiniteSequenceLengthThrows) {
    LazySequence<int> seq(BaseRule, Ordinal(1, 0));
    EXPECT_THROW(seq.GetLength(), std::logic_error);
}

TEST(LazySequenceTest, GetValidElements) {
    LazySequence<int> seq(BaseRule, Ordinal(1, 0));
    EXPECT_EQ(seq.GetByOrdinal(Ordinal(0, 0)), 1);
    EXPECT_EQ(seq.GetByOrdinal(Ordinal(0, 9)), 10);
    EXPECT_EQ(seq.Get(5), 6);
}

TEST(LazySequenceTest, GetOutOfBoundsFiniteThrows) {
    LazySequence<int> seq(BaseRule, Ordinal(0, 3));
    EXPECT_THROW(seq.GetByOrdinal(Ordinal(0, 5)), std::out_of_range);
    EXPECT_THROW(seq.Get(5), std::out_of_range);
}

// === Тесты Map() ===
TEST(LazySequenceTest, MapFiniteSequence) {
    LazySequence<int> seq(BaseRule, Ordinal(0, 3));
    auto* mapped = seq.Map<int>(MultRule);
    EXPECT_EQ(mapped->Get(0), 10);
    EXPECT_EQ(mapped->Get(2), 30);
    delete mapped;
}

TEST(LazySequenceTest, MapInfiniteSequence) {
    LazySequence<int> seq(BaseRule, Ordinal(1, 0));
    auto* mapped = seq.Map<int>(MultRule);
    EXPECT_EQ(mapped->GetOrdinalCardinality().omega0, 1);
    EXPECT_EQ(mapped->Get(100), 1010);
    delete mapped;
}

// === Тесты Skip() ===
TEST(LazySequenceTest, SkipFiniteSequence) {
    LazySequence<int> seq(BaseRule, Ordinal(0, 10));
    auto* skipped = seq.Skip(5);
    EXPECT_EQ(skipped->GetLength(), 5);
    EXPECT_EQ(skipped->Get(0), 6);
    delete skipped;
}

TEST(LazySequenceTest, SkipInfiniteSequence) {
    LazySequence<int> seq(BaseRule, Ordinal(1, 0));
    auto* skipped = seq.Skip(50);
    EXPECT_TRUE(skipped->GetOrdinalCardinality().IsInfinite());
    EXPECT_EQ(skipped->Get(0), 51);
    delete skipped;
}

TEST(LazySequenceTest, SkipMoreThanFiniteLength) {
    LazySequence<int> seq(BaseRule, Ordinal(0, 5));
    auto* skipped = seq.Skip(10);
    EXPECT_EQ(skipped->GetLength(), 0);
    EXPECT_THROW(skipped->Get(0), std::out_of_range);
    delete skipped;
}

// === Тесты конкатенации ===
TEST(LazySequenceTest, ConcatFiniteAndFinite) {
    LazySequence<int> s1(BaseRule, Ordinal(0, 3));
    LazySequence<int> s2(Seq2Rule, Ordinal(0, 2));
    auto* concat = s1.ConcatSequence(&s2);
    EXPECT_EQ(concat->GetLength(), 5);
    EXPECT_EQ(concat->Get(2), 3);
    EXPECT_EQ(concat->Get(3), 100);
    delete concat;
}

TEST(LazySequenceTest, ConcatFiniteAndInfinite) {
    LazySequence<int> s1(BaseRule, Ordinal(0, 2));
    LazySequence<int> s2(Seq2Rule, Ordinal(1, 0));
    auto* concat = s1.ConcatSequence(&s2);
    EXPECT_EQ(concat->GetOrdinalCardinality().omega0, 1);
    
    EXPECT_EQ(concat->GetOrdinalCardinality().finite, 0); 
    
    EXPECT_EQ(concat->GetByOrdinal(Ordinal(0, 2)), 100); 
    delete concat;
}

TEST(LazySequenceTest, ConcatInfiniteAndFinite) {
    LazySequence<int> s1(BaseRule, Ordinal(1, 0));
    LazySequence<int> s2(Seq2Rule, Ordinal(0, 5));
    auto* concat = s1.ConcatSequence(&s2);
    // w0 + 5
    EXPECT_EQ(concat->GetOrdinalCardinality().omega0, 1);
    EXPECT_EQ(concat->GetOrdinalCardinality().finite, 5);
    EXPECT_EQ(concat->GetByOrdinal(Ordinal(1, 0)), 100);
    delete concat;
}

TEST(LazySequenceTest, ConcatInfiniteAndInfinite) {
    LazySequence<int> s1(BaseRule, Ordinal(1, 0));
    LazySequence<int> s2(Seq2Rule, Ordinal(1, 0));
    auto* concat = s1.ConcatSequence(&s2);
    EXPECT_EQ(concat->GetOrdinalCardinality().omega0, 2);
    EXPECT_EQ(concat->GetOrdinalCardinality().finite, 0);
    EXPECT_EQ(concat->GetByOrdinal(Ordinal(1, 5)), 105);
    delete concat;
}

// === Interleave генератор ===
TEST(LazySequenceTest, InterleaveGenerators) {
    IGenerator<int>* gen1 = new RuleGenerator<int>(BaseRule, Ordinal(1, 0));
    IGenerator<int>* gen2 = new RuleGenerator<int>(Seq2Rule, Ordinal(1, 0));
    IGenerator<int>* arr[2] = {gen1, gen2};
    
    InterleaveGenerator<int> interleave(arr, 2);
    
    // Чередование: gen1(0), gen2(0), gen1(1), gen2(1) и т.д.
    EXPECT_EQ(interleave.GetAt(Ordinal(0, 0)), 1);
    EXPECT_EQ(interleave.GetAt(Ordinal(0, 1)), 100);
    EXPECT_EQ(interleave.GetAt(Ordinal(0, 2)), 2);
    EXPECT_EQ(interleave.GetAt(Ordinal(0, 3)), 101);
    
    delete gen1;
    delete gen2;
}

// === Исключение при генерации ===
TEST(LazySequenceTest, PropagatesGeneratorExceptions) {
    LazySequence<int> seq(ThrowRule, Ordinal(0, 5));
    EXPECT_THROW(seq.Get(0), std::runtime_error);
}
