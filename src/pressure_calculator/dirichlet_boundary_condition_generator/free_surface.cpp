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

FreeSurface::FreeSurface(std::unique_ptr<SurfaceDetector::Interface>&& surfaceDetector) {
    this->surfaceDetector = std::move(surfaceDetector);
}

void FreeSurface::setBoundaryCondition(Particles& particles) {
#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall) {
            pi.boundaryCondition = FluidState::Ignored;

        } else { // Fluid particles
            if (surfaceDetector->isFreeSurface(particles, pi)) {
                pi.boundaryCondition = FluidState::FreeSurface;
            } else {
                pi.boundaryCondition = FluidState::Inner;
            }
        }
    }
}
