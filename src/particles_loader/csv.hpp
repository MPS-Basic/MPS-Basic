#pragma once

#include "interface.hpp"

namespace ParticlesLoader {

/**
 * @brief Class for loading particles from a prof file
 * @details For more details on the csv file format, please refer to the @ref csv.
 */
class Csv : public Interface {
public:
    std::pair<double, Particles> load(const fs::path& path, double defaultDensity) override;
    ~Csv() override;
    Csv();
};
} // namespace ParticlesLoader
