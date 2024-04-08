#pragma once

#include "interface.hpp"

namespace PressureCalculator::DirichletBoundaryConditionGenerator {
class SpacePotentialParticle : public Interface {
private:
public:
    virtual DirichletBoundaryCondition generate(Particles& particles) override;
    ~SpacePotentialParticle() override;
};
} // namespace PressureCalculator::DirichletBoundaryConditionGenerator
