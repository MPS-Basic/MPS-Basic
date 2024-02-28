#pragma once

#include "common.hpp"
#include "mps.hpp"
#include "particles_exporter.hpp"

#include <filesystem>

/**
 * Saver class
 *
 * This class is responsible for saving the simulation results. It saves the
 * results to the file system. It uses the ParticlesExporter class to export the
 * particles to a file.
 *
 */
class Saver {
public:
    ParticlesExporter exporter;
    int fileNumber = 0;
    std::filesystem::path dir;

    Saver() = default;

    Saver(const std::filesystem::path& dir);

    void save(const MPS& mps, const double time);

private:
};
