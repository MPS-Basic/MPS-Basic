#pragma once

#include "interface.hpp"

namespace PressureCalculator::DirichletBoundaryConditionGenerator {
class SpacePotentialParticle : public Interface {
private:
public:
    virtual DirichletBoundaryCondition generate(Particles& particles) override;
    ~SpacePotentialParticle() override;
    SpacePotentialParticle() = default;
};
} // namespace PressureCalculator::DirichletBoundaryConditionGenerator
