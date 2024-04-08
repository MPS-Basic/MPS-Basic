#pragma once

#include "../../particles.hpp"
#include "../dirichlet_boundary_condition.hpp"

#include <vector>

namespace PressureCalculator::DirichletBoundaryConditionGenerator {

/**
 * @brief Interface for Dirichlet boundary condition generator
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
