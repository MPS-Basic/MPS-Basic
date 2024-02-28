#pragma once

#include "common.hpp"
#include "input.hpp"

#include <filesystem>
#include <utility>
#include <vector>

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
    Input load(const std::filesystem::path& settingPath, const std::filesystem::path& outputDirectory);

private:
    Settings loadSettingYaml(const std::filesystem::path& settingPath);

    std::pair<double, std::vector<Particle>> loadParticleProf(const std::filesystem::path& profPath);
};
