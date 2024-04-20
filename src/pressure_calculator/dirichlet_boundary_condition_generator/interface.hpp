#pragma once

#include "../../particles.hpp"
#include "../dirichlet_boundary_condition.hpp"

#include <vector>

namespace PressureCalculator::DirichletBoundaryConditionGenerator {

/**
 * @brief Interface for Dirichlet boundary condition generator
 * @details This interface is used to generate Dirichlet boundary condition for the pressure Poisson equation. The
 * function `generate` returns a Dirichlet boundary condition object and is used in the pressure calculation process.
 */
class Interface {
public:
    /**
     * @brief Generate Dirichlet boundary condition
     * @param particles Particles
     * @return Dirichlet boundary condition
     */
    virtual DirichletBoundaryCondition generate(Particles& particles) = 0;
    virtual ~Interface(){};
};

} // namespace PressureCalculator::DirichletBoundaryConditionGenerator
