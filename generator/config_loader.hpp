#include <algorithm>
#include <filesystem>
#include <yaml-cpp/yaml.h>

/// @brief get particle spacing from the settings
/// @param settingPath path to the settings
/// @return particle spacing written in the settings
double getParticleDistance(std::filesystem::path& settingPath) {
    auto yaml = YAML::LoadFile(settingPath.string());
    return yaml["particleDistance"].as<double>();
}

/// @brief get reference density from the settings
/// @param settingPath path to the settings
/// @return reference density written in the settings
double getDensity(std::filesystem::path& settingPath) {
    auto yaml = YAML::LoadFile(settingPath.string());
    return yaml["defaultDensity"].as<double>();
}

/// @brief get particle path from the settings
/// @param settingPath path to the settings
/// @return particles path written in the settings
std::filesystem::path getParticlesPath(std::filesystem::path& settingPath) {
    auto yaml = YAML::LoadFile(settingPath.string());
    return std::filesystem::weakly_canonical(settingPath.parent_path() / yaml["particlesPath"].as<std::string>());
}
