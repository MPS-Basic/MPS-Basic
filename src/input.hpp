#pragma once

#include "common.hpp"
#include "particles.hpp"
#include "settings.hpp"

#include <vector>

/**
 * @brief Represents the input data for MPS simulation.
 */
struct Input {
    Settings settings;   ///< Settings for the simulation
    Particles particles; ///< Initial particles arrangement in the simulation
    double startTime{};  ///< Start time of the simulation
};
