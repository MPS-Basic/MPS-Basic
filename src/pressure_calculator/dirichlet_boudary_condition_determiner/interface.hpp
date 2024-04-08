#pragma once

#include "../../particles.hpp"
#include "../dirichlet_boundary_condition.hpp"

#include <vector>

namespace PressureCalculator::DirichletBoundaryConditionDeterminer {

class Interface {
public:
    virtual DirichletBoundaryCondition determine(Particles& particles) = 0;
    virtual ~Interface(){};
};

} // namespace PressureCalculator::DirichletBoundaryConditionDeterminer
