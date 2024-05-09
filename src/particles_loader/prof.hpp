#pragma once

#include "interface.hpp"

namespace ParticlesLoader {

/**
 * @brief Class for loading particles from a prof file
 * @details For more details on the prof file format, please refer to the @ref profile.
 */
class Prof : public Interface {
public:
    std::pair<double, Particles> load(const fs::path& path, double defaultDensity) override;
    ~Prof() override;
    Prof();
};
} // namespace ParticlesLoader
