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

    /// @brief write the header of DataArray for the VTK format
    /// @param type type of data (e.g., Float32, Int32)
    /// @param name Name of the data array
    /// @param numberOfComponents  number of components of the data array
    /// @param format format of the data array (ascii, binary, appended)
    /// @param offset offset of the data array in the appended format
    /// @return the header of the DataArray
    std::string dataArrayBegin(
        const std::string& type,
        const std::string& name,
        int numberOfComponents,
        const std::string& format,
        size_t offset = SIZE_MAX
    ) const;
    /// @brief write the end of DataArray for the VTK format
    /// @return the end of the DataArray
    std::string dataArrayEnd() const;

    /// @brief Export the particles to a file in the VTK zlib format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    /// @param n0ForNumberDensity reference number density for the number density calculation
    void toVtuAscii(const std::filesystem::path& path, const double& time, const double& n0ForNumberDensity = 1.0);

    /// @brief Export the particles to a file in the VTK zlib format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    /// @param n0ForNumberDensity reference number density for the number density calculation
    void toVtuBinary(const std::filesystem::path& path, const double& time, const double& n0ForNumberDensity = 1.0);

public:
    Particles particles;

    /// @brief Set the particles to export to a file. This method is required before exporting.
    /// @param particles
    void setParticles(const Particles& particles);

    /// @brief Show a dialogue to check the directory and file names before exporting the initial particles in generators.
    /// @details This method will create a directory if it does not exist.
    /// If the directory exists, it will show the files in the directory and ask for confirmation to overwrite them.
    /// @param parentPath path to the parent directory where the files will be exported
    void generatorDialogue(const std::filesystem::path& parentPath);     

    /// @brief Export the particles to a file in the Prof format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    void toProf(const std::filesystem::path& path, const double& time);

    /// @brief Export the particles to a file in the VTK zlib format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    /// @param n0ForNumberDensity reference number density for the number density calculation
    void toVtu(
        const std::filesystem::path& path,
        const double& time,
        const double& n0ForNumberDensity = 1.0,
        const bool binary                = false
    );

    /// @brief Export the particles to a file in the CSV format.
    /// @param path path to the file to write
    /// @param time current time in the simulation
    void toCsv(const std::filesystem::path& path, const double& time);
};
