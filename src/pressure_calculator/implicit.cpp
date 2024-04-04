#include "implicit.hpp"

#include "../refvalues.hpp"
#include "../weight.hpp"

#include <iostream>
#include <queue>

using PressureCalculator::Implicit;
using std::cerr;
using std::endl;

Implicit::Implicit(
    int dimension,
    double particleDistance,
    double reForNumberDensity,
    double reForLaplacian,
    double dt,
    double fluidDensity,
    double compressibility,
    double relaxationCoefficient
) {
    auto refValuesForNumberDensity = RefValues(dimension, particleDistance, reForNumberDensity);
    auto refValuesForLaplacian     = RefValues(dimension, particleDistance, reForLaplacian);
    this->pressurePoissonEquation  = PressurePoissonEquation(
        dimension,
        dt,
        relaxationCoefficient,
        fluidDensity,
        compressibility,
        refValuesForNumberDensity.n0,
        refValuesForLaplacian.n0,
        refValuesForLaplacian.lambda,
        reForLaplacian,
        reForNumberDensity
    );
}

std::vector<double> Implicit::calc(const std::vector<Particle>& particles) {
    // Boundary condition: Pressure update is performed only for inner particles.
    auto isPressureUpdateTarget = [](const Particle& p) { return p.boundaryCondition == FluidState::Inner; };

    this->pressurePoissonEquation.setup(particles, isPressureUpdateTarget);
    this->pressure = this->pressurePoissonEquation.solve();
    removeNegativePressure();

    return this->pressure;
}

Implicit::~Implicit() {
}

void Implicit::removeNegativePressure() {
#pragma omp parallel for
    for (auto& p : pressure) {
        if (p < 0) {
            p = 0;
        }
    }
}
