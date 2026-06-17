#include <gtest/gtest.h>
#include "../src/MyUtils.hpp"

using namespace my_utils;

TEST(OrdinalTest, Calculations) {
    Ordinal c;
    EXPECT_EQ(c.omega0, 0);
    
    Ordinal inf(2, 5); // 2*w0 + 5
    EXPECT_TRUE(inf.IsInfinite());

    Ordinal b(1, 3);
    Ordinal res = inf + b; // (2*w0 + 5) + (1*w0 + 3) = 3*w0 + 3
    EXPECT_EQ(res.omega0, 3);
    EXPECT_EQ(res.finite, 3);
}
