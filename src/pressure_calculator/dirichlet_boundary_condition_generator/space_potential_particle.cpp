#include "space_potential_particle.hpp"

using PressureCalculator::DirichletBoundaryConditionGenerator::SpacePotentialParticle;

DirichletBoundaryCondition SpacePotentialParticle::generate(Particles& particles) {
    DirichletBoundaryCondition dirichletBoundaryCondition;

    // ParticleType が SPP, Ghost, DummyWall で 0
    for (const auto& p : particles) {
        if (p.type == ParticleType::SPP || p.type == ParticleType::Ghost || p.type == ParticleType::DummyWall) {
            dirichletBoundaryCondition.set(p.id, 0.0);
        }
    }

    return dirichletBoundaryCondition;
}

SpacePotentialParticle::~SpacePotentialParticle() {
}
