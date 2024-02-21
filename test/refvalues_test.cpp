#include "refvalues.hpp"
#include "weight.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(RefValuesTest, NumberDensityForGradientTest2d) {
	double l0                  = 0.1;
	double re_forNumberDensity = 2.5 * l0;
	double re_forGradient      = 1.5 * l0;
	double re_forLaplacian     = 3.1 * l0;
	RefValues ref;
	ref.calc(2, l0, re_forNumberDensity, re_forGradient, re_forLaplacian);

	double n0_exact = 4.0 * weight(l0, re_forGradient) + 4.0 * weight(sqrt(2.0) * l0, re_forGradient);

	EXPECT_DOUBLE_EQ(ref.n0_forGradient, n0_exact);
}
TEST(RefValuesTest, NumberDensityForGradientTest3d) {
	double l0                  = 0.1;
	double re_forNumberDensity = 2.5 * l0;
	double re_forGradient      = 1.5 * l0;
	double re_forLaplacian     = 3.1 * l0;
	RefValues ref;
	ref.calc(3, l0, re_forNumberDensity, re_forGradient, re_forLaplacian);

	double n0_exact = 6.0 * weight(l0, re_forGradient) + 12.0 * weight(sqrt(2.0) * l0, re_forGradient);
	EXPECT_DOUBLE_EQ(ref.n0_forGradient, n0_exact);
}
TEST(RefValuesTest, NumberDensityForLaplacianTest2d) {
	double l0                  = 0.1;
	double re_forNumberDensity = 2.5 * l0;
	double re_forGradient      = 2.1 * l0;
	double re_forLaplacian     = 1.5 * l0;
	RefValues ref;
	ref.calc(2, l0, re_forNumberDensity, re_forGradient, re_forLaplacian);

	double n0_exact = 4.0 * weight(l0, re_forLaplacian) + 4.0 * weight(sqrt(2.0) * l0, re_forLaplacian);
	EXPECT_DOUBLE_EQ(ref.n0_forLaplacian, n0_exact);
}
TEST(RefValuesTest, NumberDensityForLaplacianTest3d) {
	double l0                  = 0.1;
	double re_forNumberDensity = 2.5 * l0;
	double re_forGradient      = 2.1 * l0;
	double re_forLaplacian     = 1.5 * l0;
	RefValues ref;
	ref.calc(3, l0, re_forNumberDensity, re_forGradient, re_forLaplacian);

	double n0_exact = 6.0 * weight(l0, re_forLaplacian) + 12.0 * weight(sqrt(2.0) * l0, re_forLaplacian);
	EXPECT_DOUBLE_EQ(ref.n0_forLaplacian, n0_exact);
}