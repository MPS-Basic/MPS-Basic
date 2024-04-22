#pragma once

#include "../particles.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace ParticlesLoader {
class Interface {
public:
    /**
     * @brief Load particles
     * @return Pair of simulation start time and particles
     */
    virtual std::pair<double, Particles> load(const fs::path& path) = 0;
    virtual ~Interface()                                            = default;
};
} // namespace ParticlesLoader
