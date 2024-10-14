#include "loader.hpp"

#include <filesystem>
#include <gtest/gtest.h>
TEST(LoaderTest, LoadDambreakConfiguration) {
    Loader loader;
    auto outputDirectory = std::filesystem::temp_directory_path();
    auto input           = loader.load("input/hydrostatic/settings.yml", outputDirectory);
    auto settings        = input.settings;

    // Check the settings
    EXPECT_EQ(settings.dim, 2);
    EXPECT_DOUBLE_EQ(settings.particleDistance, 0.012);
    EXPECT_DOUBLE_EQ(settings.dt, 0.002);
    EXPECT_DOUBLE_EQ(settings.endTime, 10.0);
    // domain
    EXPECT_DOUBLE_EQ(settings.domain.xMin, -0.25);
    EXPECT_DOUBLE_EQ(settings.domain.xMax, 0.25);
    EXPECT_DOUBLE_EQ(settings.domain.yMin, -0.05);
    EXPECT_DOUBLE_EQ(settings.domain.yMax, 0.6);
    EXPECT_DOUBLE_EQ(settings.domain.zMin, 0.0);
    EXPECT_DOUBLE_EQ(settings.domain.zMax, 0.0);
}
