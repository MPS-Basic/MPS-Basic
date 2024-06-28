#include "particle.hpp"

Particle::Particle(int id, ParticleType type, Eigen::Vector3d pos, Eigen::Vector3d vel, double density, int fluidType) {
    this->id        = id;
    this->type      = type;
    this->position  = pos;
    this->velocity  = vel;
    this->density   = density;
    this->fluidType = fluidType;
}

double Particle::inverseDensity() const {
    switch (type) {
    case ParticleType::Fluid:
        return 1 / this->density;

    case ParticleType::Wall:
    case ParticleType::DummyWall:
    case ParticleType::Ghost:
    default:
        return 0;
    }
}
