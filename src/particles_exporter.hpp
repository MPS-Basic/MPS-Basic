#pragma once

#include "common.hpp"
#include "particle.hpp"

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
public:
    std::vector<Particle> particles;

    void setParticles(const std::vector<Particle>& particles);
    void toProf(const std::filesystem::path& path, const double& time);
    void toVtu(const std::filesystem::path& path, const double& time, const double& n0ForNumberDensity = 1.0);

private:
    static void dataArrayBegin(
        std::ofstream& ofs, const std::string& numberOfComponents, const std::string& type, const std::string& name
    );
    static void dataArrayEnd(std::ofstream& ofs);
};
