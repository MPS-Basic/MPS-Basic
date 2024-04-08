#include "free_surface.hpp"

#include "../../refvalues.hpp"

using PressureCalculator::DirichletBoundaryCondition;
using PressureCalculator::DirichletBoundaryConditionGenerator::FreeSurface;

DirichletBoundaryCondition FreeSurface::generate(Particles& particles) {
    setBoundaryCondition(particles);

    // Boundary condition: Particles other than inner particles set pressure to 0
    DirichletBoundaryCondition dirichletBoundaryCondition;
    for (const auto& p : particles) {
        if (p.boundaryCondition != FluidState::Inner) {
            dirichletBoundaryCondition.set(p.id, 0.0);
        }
    }

    return dirichletBoundaryCondition;
}

FreeSurface::~FreeSurface() {
}

FreeSurface::FreeSurface(int dim, double particleDistance, double re_forNumberDensity, double beta) {
    auto refValues = RefValues(dim, particleDistance, re_forNumberDensity);
    this->n0       = refValues.n0;
    this->beta     = beta;
}

void FreeSurface::setBoundaryCondition(Particles& particles) {
#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall) {
            pi.boundaryCondition = FluidState::Ignored;

        } else { // Fluid particles
            if (isFreeSurface(pi)) {
                pi.boundaryCondition = FluidState::FreeSurface;
            } else {
                pi.boundaryCondition = FluidState::Inner;
            }
        }
    }
}

bool FreeSurface::isFreeSurface(const Particle& pi) {
    return pi.numberDensity < beta * n0;
}
