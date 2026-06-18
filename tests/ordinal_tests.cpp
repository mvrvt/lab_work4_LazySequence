#include <gtest/gtest.h>
#include "../src/MyUtils.hpp"

using namespace my_utils;

// === Тесты конструкторов ===
TEST(OrdinalTest, DefaultConstructor) {
    Ordinal c;
    EXPECT_EQ(c.omega0, 0);
    EXPECT_EQ(c.finite, 0);
    EXPECT_FALSE(c.IsInfinite());
}

TEST(OrdinalTest, FiniteConstructor) {
    Ordinal c(0, 10);
    EXPECT_EQ(c.omega0, 0);
    EXPECT_EQ(c.finite, 10);
    EXPECT_FALSE(c.IsInfinite());
}

TEST(OrdinalTest, InfiniteConstructor) {
    Ordinal c(5, 42);
    EXPECT_EQ(c.omega0, 5);
    EXPECT_EQ(c.finite, 42);
    EXPECT_TRUE(c.IsInfinite());
}

// === Тесты операторов сравнения===
TEST(OrdinalTest, EqualityFinite) {
    EXPECT_TRUE(Ordinal(0, 5) == Ordinal(0, 5));
    EXPECT_FALSE(Ordinal(0, 5) == Ordinal(0, 6));
}

TEST(OrdinalTest, EqualityInfinite) {
    EXPECT_TRUE(Ordinal(2, 5) == Ordinal(2, 5));
    EXPECT_FALSE(Ordinal(2, 5) == Ordinal(3, 5));
}

TEST(OrdinalTest, LessThanFinite) {
    EXPECT_TRUE(Ordinal(0, 5) < Ordinal(0, 10));
    EXPECT_FALSE(Ordinal(0, 10) < Ordinal(0, 5));
}

TEST(OrdinalTest, LessThanInfinite) {
    EXPECT_TRUE(Ordinal(1, 100) < Ordinal(2, 0));
    EXPECT_TRUE(Ordinal(2, 5) < Ordinal(2, 6));
    EXPECT_FALSE(Ordinal(3, 0) < Ordinal(2, 100));
}

TEST(OrdinalTest, GreaterOrEqual) {
    EXPECT_TRUE(Ordinal(2, 5) >= Ordinal(2, 5));
    EXPECT_TRUE(Ordinal(3, 0) >= Ordinal(2, 100));
    EXPECT_FALSE(Ordinal(1, 5) >= Ordinal(1, 6));
}

// === Тесты сложения ===
TEST(OrdinalTest, AddFiniteToFinite) {
    Ordinal res = Ordinal(0, 10) + Ordinal(0, 5);
    EXPECT_EQ(res.omega0, 0);
    EXPECT_EQ(res.finite, 15);
}

TEST(OrdinalTest, AddInfiniteToFinite) {
    Ordinal res = Ordinal(0, 10) + Ordinal(2, 5); // 10 + (w0*2 + 5) = w0*2 + 5
    EXPECT_EQ(res.omega0, 2);
    EXPECT_EQ(res.finite, 5);
}

TEST(OrdinalTest, AddFiniteToInfinite) {
    Ordinal res = Ordinal(2, 5) + Ordinal(0, 10); // (w0*2 + 5) + 10 = w0*2 + 15
    EXPECT_EQ(res.omega0, 2);
    EXPECT_EQ(res.finite, 15);
}

TEST(OrdinalTest, AddInfiniteToInfinite) {
    Ordinal res = Ordinal(1, 100) + Ordinal(2, 5); // (w0*1 + 100) + (w0*2 + 5) = w0*3 + 5
    EXPECT_EQ(res.omega0, 3);
    EXPECT_EQ(res.finite, 5);
}

// === Тесты вычитания ===
TEST(OrdinalTest, SubtractFinite) {
    Ordinal res = Ordinal(0, 10) - Ordinal(0, 3);
    EXPECT_EQ(res.omega0, 0);
    EXPECT_EQ(res.finite, 7);
}

TEST(OrdinalTest, SubtractInfiniteSameOmega) {
    Ordinal res = Ordinal(2, 10) - Ordinal(2, 0);
    EXPECT_EQ(res.omega0, 0);
    EXPECT_EQ(res.finite, 10);
}

TEST(OrdinalTest, SubtractInfiniteDiffOmega) {
    Ordinal res = Ordinal(5, 10) - Ordinal(2, 0);
    EXPECT_EQ(res.omega0, 3);
    EXPECT_EQ(res.finite, 10);
}
