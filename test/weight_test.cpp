#include "weight.hpp"

#include <gtest/gtest.h>

TEST(WeightTest, WeightTest) {
    double re = 1.0;

    for (WeightType wt : {WeightType::SemiImplicit, WeightType::Explicit, WeightType::ExplicitGrad}) {
        Weight weight(wt);

        double dis = 0.5;
        if (wt == WeightType::SemiImplicit) {
            EXPECT_DOUBLE_EQ(weight.weightFn(dis, re), 1.0);
        } else if (wt == WeightType::Explicit) {
            EXPECT_DOUBLE_EQ(weight.weightFn(dis, re), 0.5);
        } else if (wt == WeightType::ExplicitGrad) {
            EXPECT_DOUBLE_EQ(weight.weightFn(dis, re), 1.5);
        }

        dis = 1.0;
        EXPECT_DOUBLE_EQ(weight.weightFn(dis, re), 0.0);

        dis = 1.5;
        EXPECT_DOUBLE_EQ(weight.weightFn(dis, re), 0.0);
    }
}
