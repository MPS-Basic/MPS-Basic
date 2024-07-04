#pragma once

#include "../particles.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace ParticlesLoader {
class Interface {
public:
    /**
     * @brief Load particles
     * @param path Path to the particles file
     * @param defaultDensity Default density of particles
     * @return Pair of simulation start time and particles
     */
    virtual std::pair<double, Particles> load(const fs::path& path, double defaultDensity) = 0;
    virtual ~Interface()                                                                   = default;
};
} // namespace ParticlesLoader
