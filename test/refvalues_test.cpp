#include "refvalues.hpp"
#include "weight.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(RefValuesTest, test2d) {
    double l0 = 0.1;
    double re = 1.5 * l0;
    RefValues ref(2, l0, re);

    double n0_exact = 4.0 * weight(l0, re) + 4.0 * weight(sqrt(2.0) * l0, re);
    double lambda_exact =
        (4.0 * (l0 * l0) * weight(l0, re) + 4.0 * (2.0 * l0 * l0) * weight(sqrt(2.0) * l0, re)) / n0_exact;

    EXPECT_DOUBLE_EQ(ref.n0, n0_exact);
    EXPECT_DOUBLE_EQ(ref.lambda, lambda_exact);
}
TEST(RefValuesTest, test3d) {
    double l0 = 0.1;
    double re = 1.5 * l0;
    RefValues ref(3, l0, re);

    double n0_exact = 6.0 * weight(l0, re) + 12.0 * weight(sqrt(2.0) * l0, re);
    double lambda_exact =
        (6.0 * (l0 * l0) * weight(l0, re) + 12.0 * (2.0 * l0 * l0) * weight(sqrt(2.0) * l0, re)) / n0_exact;

    EXPECT_DOUBLE_EQ(ref.n0, n0_exact);
    EXPECT_DOUBLE_EQ(ref.lambda, lambda_exact);
}