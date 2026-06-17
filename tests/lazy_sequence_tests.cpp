#include <gtest/gtest.h>
#include "../src/LazySequence.hpp"
#include "../src/MyUtils.hpp"

using namespace my_utils;

int TestRule(int index) { return index + 1; }
int MultRule(const int& x) { return x * 2; }
int SecRule(int index) { return 100 + index; }

TEST(LazySequenceTest, Pipelines) {
    LazySequence<int> seq1(TestRule, Ordinal(1, 0), 5);
    EXPECT_EQ(seq1.GetByOrdinal(Ordinal(0, 0)), 1);

    LazySequence<int>* mapped = seq1.Map<int>(MultRule);
    EXPECT_EQ(mapped->GetByOrdinal(Ordinal(0, 1)), 4);

    LazySequence<int> seq2(SecRule, Ordinal(1, 0), 5);
    LazySequence<int>* concat = seq1.ConcatSequence(&seq2);

    // Главная проверка трансфинитных ординалов с доски препода!
    EXPECT_EQ(concat->GetOrdinalCardinality().omega0, 2);
    EXPECT_EQ(concat->GetByOrdinal(Ordinal(1, 5)), 105);

    delete mapped;
    delete concat;
}
