#include <gtest/gtest.h>
#include "particles_exporter.hpp"

TEST(ParticlesExporterTest, SetParticles) {
    constexpr size_t particleSize = 100;

    ParticlesExporter exporter;
    Particles particles;
    for (size_t i = 0; i < particleSize; i++) {
        auto r_i = Eigen::Vector3d::Zero();
        auto u_i = Eigen::Vector3d::Zero();
        particles.add(Particle(i, ParticleType::Fluid, r_i, u_i, 1.0, 0));
    }

    exporter.setParticles(particles);
    EXPECT_EQ(exporter.particles.size(), particles.size());
}
TEST(ParticlesExporterTest, ToProf) {
    constexpr size_t particleSize = 100;

    ParticlesExporter exporter;
    Particles particles;
    for (size_t i = 0; i < particleSize; i++) {
        auto r_i = Eigen::Vector3d::Zero();
        auto u_i = Eigen::Vector3d::Zero();
        particles.add(Particle(i, ParticleType::Fluid, r_i, u_i, 1.0, 0));
    }

    exporter.setParticles(particles);
    const std::filesystem::path path = "test.prof";
    const double time = 0.0;
    exporter.toProf(path, time);
    EXPECT_TRUE(std::filesystem::exists(path));
    std::filesystem::remove(path);
}
TEST(ParticlesExporterTest, ToVtu) {
    constexpr size_t particleSize = 100;

    ParticlesExporter exporter;
    Particles particles;
    for (size_t i = 0; i < particleSize; i++) {
        auto r_i = Eigen::Vector3d::Zero();
        auto u_i = Eigen::Vector3d::Zero();
        particles.add(Particle(i, ParticleType::Fluid, r_i, u_i, 1.0, 0));
    }

    exporter.setParticles(particles);
    const std::filesystem::path path = "test.vtu";
    const double time = 0.0;
    exporter.toVtu(path, time);
    EXPECT_TRUE(std::filesystem::exists(path));
    std::filesystem::remove(path);
}