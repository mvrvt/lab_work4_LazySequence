#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/MyUtils.hpp"

using namespace my_utils;

// Тесты на конструкторы
TEST(OrdinalTest, DefaultConstructor) {
    Ordinal c;
    EXPECT_EQ(c.omega0, 0);
    EXPECT_EQ(c.finite, 0);
    EXPECT_FALSE(c.IsInfinite());
}

TEST(OrdinalTest, FiniteConstructor) {
    Ordinal c(0, 10); // 0 бесконечностей, 10 конечных
    EXPECT_EQ(c.omega0, 0);
    EXPECT_EQ(c.finite, 10);
    EXPECT_FALSE(c.IsInfinite());
}

TEST(OrdinalTest, InfiniteConstructor) {
    Ordinal c(2, 5); // 2*w0 + 5
    EXPECT_EQ(c.omega0, 2);
    EXPECT_EQ(c.finite, 5);
    EXPECT_TRUE(c.IsInfinite());
}

// Проверка операторов сравнения
TEST(OrdinalTest, Equality) {
    Ordinal a(1, 5);
    Ordinal b(1, 5);
    EXPECT_TRUE(a == b);
}

TEST(OrdinalTest, Inequality) {
    Ordinal a(1, 5);
    Ordinal b(1, 6);
    Ordinal c(2, 5);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a < c); // W0*1 < W0*2
}

// Проверка сложения
TEST(OrdinalTest, AdditionFinite) {
    Ordinal a(0, 10);
    Ordinal b(0, 5);
    Ordinal result = a + b;
    EXPECT_EQ(result.omega0, 0);
    EXPECT_EQ(result.finite, 15);
}

TEST(OrdinalTest, AdditionInfinite) {
    // В ординальной арифметике (w0*A + B) + (w0*C + D) = w0*(A+C) + D,
    // потому что конечный остаток B поглощается следующей бесконечностью!
    Ordinal a(1, 5);
    Ordinal b(2, 3);
    Ordinal result = a + b;
    EXPECT_EQ(result.omega0, 3);
    EXPECT_EQ(result.finite, 3);
}

// Проверка вычитания (для индексов)
TEST(OrdinalTest, SubtractionFinite) {
    Ordinal a(0, 10);
    Ordinal b(0, 3);
    Ordinal result = a - b;
    EXPECT_EQ(result.omega0, 0);
    EXPECT_EQ(result.finite, 7);
}

TEST(OrdinalTest, SubtractionInfinite) {
    Ordinal a(3, 10);
    Ordinal b(1, 0);
    Ordinal result = a - b;
    EXPECT_EQ(result.omega0, 2);
    EXPECT_EQ(result.finite, 10);
}
