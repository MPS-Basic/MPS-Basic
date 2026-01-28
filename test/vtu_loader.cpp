#include "../src/particle.hpp"
#include "../src/particles.hpp"
#include "../src/particles_exporter.hpp"
#include "../src/particles_loader/vtu.hpp"

#include <filesystem>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class VtuTest : public ::testing::Test {
protected:
    fs::path testFilePath;

    void SetUp() override {
        testFilePath = "test/temp_file_for_vtu_loader.vtu";
    }

    // void TearDown() override {
    //     if (fs::exists(testFilePath)) {
    //         fs::remove(testFilePath);
    //     }
    // }
};

TEST_F(VtuTest, LoadValidVtuFile) {
    // Particles data for testing
    Particles particles;
    particles.add(
        Particle(0, ParticleType::Fluid, Eigen::Vector3d(0.0, 0.0, 0.0), Eigen::Vector3d(0.1, 0.1, 0.1), 2000.0, 1)
    );
    particles.add(
        Particle(1, ParticleType::Fluid, Eigen::Vector3d(1.0, 1.0, 1.0), Eigen::Vector3d(0.2, 0.2, 0.2), -1.0, 2)
    );

    // Generate VTU file for testing
    ParticlesExporter exporter;
    exporter.setParticles(particles);
    exporter.toVtu(testFilePath, 0.0, 1.0); // Ascii format

    // Load the VTU file
    ParticlesLoader::Vtu vtuLoader;
    double defaultDensity             = 1000.0;
    auto [startTime, loadedParticles] = vtuLoader.load(testFilePath, defaultDensity);

    // Test
    EXPECT_DOUBLE_EQ(startTime, 0.0);
    ASSERT_EQ(loadedParticles.size(), 2);

    auto particle1 = loadedParticles[0];
    EXPECT_EQ(particle1.id, 0);
    EXPECT_EQ(particle1.type, ParticleType::Fluid);
    EXPECT_EQ(particle1.position, Eigen::Vector3d(0.0, 0.0, 0.0));
    EXPECT_EQ(particle1.velocity, Eigen::Vector3d(0.1, 0.1, 0.1));

    auto particle2 = loadedParticles[1];
    EXPECT_EQ(particle2.id, 1);
    EXPECT_EQ(particle2.type, ParticleType::Fluid);
    EXPECT_EQ(particle2.position, Eigen::Vector3d(1.0, 1.0, 1.0));
    EXPECT_EQ(particle2.velocity, Eigen::Vector3d(0.2, 0.2, 0.2));
}
