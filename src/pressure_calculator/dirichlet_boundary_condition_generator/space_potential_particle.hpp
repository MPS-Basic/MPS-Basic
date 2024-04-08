#pragma once

#include "interface.hpp"

namespace PressureCalculator::DirichletBoundaryConditionGenerator {
class SpacePotentialParticle {
private:
public:
    DirichletBoundaryCondition generate(Particles& particles) override;
    ~SpacePotentialParticle() override;
};
} // namespace PressureCalculator::DirichletBoundaryConditionGenerator
