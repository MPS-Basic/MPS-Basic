#pragma once

#include "../../particle.hpp"
#include "interface.hpp"

namespace PressureCalculator::DirichletBoundaryConditionDeterminer {
class FreeSurface : public Interface {
public:
    DirichletBoundaryCondition determine(Particles& particles) override;
    ~FreeSurface() override;
    FreeSurface(int dim, double particleDistance, double re_forNumberDensity, double beta);

private:
    double n0;
    double beta;
    void setBoundaryCondition(Particles& particles);
    bool isFreeSurface(const Particle& pi);
};
}; // namespace PressureCalculator::DirichletBoundaryConditionDeterminer
