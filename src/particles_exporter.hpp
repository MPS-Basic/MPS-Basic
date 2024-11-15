#pragma once

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
private:
    /// @brief detect the endian of the system
    /// @details
    /// Some CPUs are big endian, and others are little endian.
    /// The endian of the system is detected by checking the first byte of the
    /// 32-bit integer 1. If the first byte is 0, the system is big endian.
    /// @return if the system is big endian, return true, otherwise false
    bool isBigEndian() const;

public:
    Particles particles;

    /// @brief Set the particles to export to a file. This method is required before exporting.
    /// @param particles
    void setParticles(const Particles& particles);

    /// @brief Export the particles to a file in the Prof format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    void toProf(const std::filesystem::path& path, const double& time);

    /// @brief Export the particles to a file in the VTK zlib format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    /// @param n0ForNumberDensity reference number density for the number density calculation
    void toVtu(const std::filesystem::path& path, const double& time, const double& n0ForNumberDensity = 1.0);
    /// @brief Export the particles to a file in the CSV format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    void toCsv(const std::filesystem::path& path, const double& time);
};
