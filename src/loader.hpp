#pragma once

#include "common.hpp"
#include "input.hpp"
#include "particles.hpp"
#include "particles_loader/interface.hpp"

#include <filesystem>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

/**
 * @brief Class for loading setting file and particle file
 *
 * @details This class is responsible for loading the input. It loads the
 * settings and the particles from the file system. It uses the YAML library to
 * load the settings from a YAML file. It also uses the std::filesystem library
 * to load the particles from a file.
 */
class Loader {
public:
    Loader() = default;

    /**
     * @brief Load the setting file and the particle file
     * @note This function copy the setting file and the particle file to the
     * output directory.
     * @param settingPath Path to the setting file
     * @param outputDirectory Path to the output directory
     * @return Input object
     */
    Input load(const fs::path& settingPath, const fs::path& outputDirectory);

private:
    std::unique_ptr<ParticlesLoader::Interface> particlesLoader;

    std::unique_ptr<ParticlesLoader::Interface> getParticlesLoader(const fs::path& particlesPath);
    void copyInputFileToOutputDirectory(const fs::path& inputFilePath, const fs::path& outputDirectory);
    Settings loadSettingYaml(const fs::path& settingPath);
};
