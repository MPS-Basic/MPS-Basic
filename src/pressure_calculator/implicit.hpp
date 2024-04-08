#pragma once

#include "../particles.hpp"
#include "../refvalues.hpp"
#include "dirichlet_boudary_condition_determiner/interface.hpp"
#include "interface.hpp"
#include "pressure_poisson_equation.hpp"

#include <Eigen/Sparse>
#include <vector>

namespace PressureCalculator {

class Implicit : public Interface {
public:
    /**
     * @brief calculate pressure
     * @param particles particles
     */
    std::vector<double> calc(Particles& particles) override;
    ~Implicit() override;

    Implicit(
        int dimension,
        double particleDistance,
        double re_forGradient,
        double re_forLaplacian,
        double dt,
        double fluidDensity,
        double compressibility,
        double relaxationCoefficient,
        std::unique_ptr<DirichletBoundaryConditionDeterminer::Interface> dirichletBoundaryConditionDeterminer
    );

private:
    Particles particles;
    std::vector<double> pressure; ///< Solution of pressure calculation
    std::unique_ptr<DirichletBoundaryConditionDeterminer::Interface> dirichletBoundaryConditionDeterminer;
    PressurePoissonEquation pressurePoissonEquation;

    /**
     * @brief remove negative pressure for stability
     */
    void removeNegativePressure();
};

} // namespace PressureCalculator
