#pragma once

#include "interface.hpp"

namespace ParticlesLoader {

/**
 * @brief Class for loading particles from a VTU file
 * @details This class handles the VTU file format for particle data.
 */
class Vtu : public Interface {
public:
    /**
     * @brief Load particles from a VTU file
     * @param path Path to the VTU file
     * @param defaultDensity Default density to use if not specified in the file
     * @return Pair of simulation start time and particles
     */
    std::pair<double, Particles> load(const fs::path& path, double defaultDensity) override;

    ~Vtu() override;
    Vtu();
};
} // namespace ParticlesLoader
