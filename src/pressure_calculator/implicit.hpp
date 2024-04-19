#pragma once

#include "../particles.hpp"
#include "../refvalues.hpp"
#include "dirichlet_boundary_condition_generator/interface.hpp"
#include "interface.hpp"
#include "pressure_poisson_equation.hpp"

#include <Eigen/Sparse>
#include <memory>
#include <vector>

namespace PressureCalculator {

/**
 * @brief Class for implicit pressure calculation
 * @details This class has a Dirichlet boundary condition generator and a pressure Poisson solver. The generator is used
 * within the pressure calculation process and the generated boundary condition is attached to the pressure Poisson
 * equation.
 */
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
        std::unique_ptr<DirichletBoundaryConditionGenerator::Interface>&& dirichletBoundaryConditionGenerator
    );

private:
    Particles particles;
    std::vector<double> pressure; ///< Solution of pressure calculation
    std::unique_ptr<DirichletBoundaryConditionGenerator::Interface> dirichletBoundaryConditionGenerator;
    PressurePoissonEquation pressurePoissonEquation;

    /**
     * @brief remove negative pressure for stability
     */
    void removeNegativePressure();
};

} // namespace PressureCalculator
