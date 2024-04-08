#pragma once

#include "../../particle.hpp"
#include "interface.hpp"

namespace PressureCalculator::DirichletBoundaryConditionGenerator {
/**
 * @brief Dirichlet Boundary Condition Generator that sets the free surface pressure to 0
 */
class FreeSurface : public Interface {
public:
    /**
     * @brief Generate Dirichlet boundary condition
     * @param particles Particles
     * @return Dirichlet boundary condition
     */
    DirichletBoundaryCondition generate(Particles& particles) override;
    ~FreeSurface() override;

    /**
     * @brief Constructor
     * @param dim Dimension
     * @param particleDistance Particle distance
     * @param re_forNumberDensity Reference number density
     * @param beta Threshold ratio of particle number density for surface particle determination
     */
    FreeSurface(int dim, double particleDistance, double re_forNumberDensity, double beta);

private:
    double n0;
    double beta;
    void setBoundaryCondition(Particles& particles);
    bool isFreeSurface(const Particle& pi);
};
}; // namespace PressureCalculator::DirichletBoundaryConditionGenerator
