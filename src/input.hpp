#pragma once

#include "settings.hpp"
#include "particle.hpp"
#include <vector>

/**
 * @brief Represents the input data for MPS simulation.
 * */ 
struct Input {
    Settings settings; ///< Settings for the simulation
    std::vector<Particle> particles; ///< Initial particles arrangement in the simulation
    double initialTime; ///< Initial time of the simulation
};
