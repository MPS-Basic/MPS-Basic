#pragma once

#include "../particles.hpp"
#include "interface.hpp"

#include <vector>

namespace PressureCalculator {

/**
 * @brief Class for explicit pressure calculation
 * @details This class calculates pressure explicitly.
 */
class Explicit : public Interface {
public:
    /**
     * @brief calculate pressure
     * @param particles particles
     */
    std::vector<double> calc(Particles& particles) override;
    ~Explicit() override;

    Explicit(double n0, double soundSpeed, int dimension, double particleDistance);

private:
    double n0;
    double soundSpeed;
};

} // namespace PressureCalculator
