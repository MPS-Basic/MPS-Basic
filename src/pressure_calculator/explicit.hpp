#pragma once

#include "../particles.hpp"
#include "interface.hpp"

#include <vector>

namespace PressureCalculator {

class Explicit : public Interface {
public:
    /**
     * @brief calculate pressure
     * @param particles particles
     */
    std::vector<double> calc(const Particles& particles) override;
    ~Explicit() override;

    Explicit(double fluidDensity, double n0, double soundSpeed, int dimension, double particleDistance);

private:
    double fluidDensity;
    double n0;
    double soundSpeed;
};

} // namespace PressureCalculator
