#pragma once

#include <gtest/gtest.h>
#include "common.hpp"
#include "particles.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

/**
 * ParticlesExporter class
 *
 * This class is responsible for exporting the particles to a file. It can
 * export the particles to certain file formats. If you want to add a new
 * file format, you can add a new method to this class. It only requires
 * the particles (and the time) to export the particles to a file.
 *
 */
class ParticlesExporter {
    FRIEND_TEST(ParticlesExporterTest, isLittleEndian);
public:
    Particles particles;

    void setParticles(const Particles& particles);
    void toProf(const std::filesystem::path& path, const double& time);
    void toVtu(const std::filesystem::path& path, const double& time, const double& n0ForNumberDensity = 1.0);
    void toCsv(const std::filesystem::path& path, const double& time);
};
