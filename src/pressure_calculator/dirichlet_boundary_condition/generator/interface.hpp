#pragma once

#include "../../../particles.hpp"
#include "../dirichlet_boundary_condition.hpp"

#include <vector>

namespace PressureCalculator::DirichletBoundaryCondition::Generator {
class Interface {
public:
    virtual DirichletBoundaryCondition generate(Particles& particles) = 0;
    virtual ~Interface(){};
};

} // namespace PressureCalculator::DirichletBoundaryCondition::Generator
