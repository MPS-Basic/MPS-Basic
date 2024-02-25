#include "weight.hpp"
#include <gtest/gtest.h>

TEST(WeightTest, WeightTest) {
    double re  = 1.0;
    double dis = 0.5;
    EXPECT_DOUBLE_EQ(weight(dis, re), 1.0);
    dis = 1.0;
    EXPECT_DOUBLE_EQ(weight(dis, re), 0.0);
    dis = 1.5;
    EXPECT_DOUBLE_EQ(weight(dis, re), 0.0);
}