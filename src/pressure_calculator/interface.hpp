#pragma once

#include "../particle.hpp"

#include <vector>

namespace PressureCalculator {

/**
 * @brief Pressure calculator interface
 *
 * @details Interface for pressure calculator. This interface is used to
 * calculate pressure in the MPS method.
 */
class Interface {
public:
    /**
     * @brief calculate pressure
     * @param particles particles
     *
     * @return pressures of particles
     */
    virtual std::vector<double> calc(const std::vector<Particle>& particles) = 0;

    /**
     * @brief destructor
     */
    virtual ~Interface(){};
};

} // namespace PressureCalculator
