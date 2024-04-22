#include "particle.hpp"

Particle::Particle(
    int id, ParticleType type, Eigen::Vector3d pos, Eigen::Vector3d vel, std::string fluidType = std::string()
) {
    this->id        = id;
    this->type      = type;
    this->position  = pos;
    this->velocity  = vel;
    this->fluidType = fluidType;
}

double Particle::inverseDensity(double& density) const {
    switch (type) {
    case ParticleType::Ghost:
        return std::numeric_limits<double>::infinity();

    case ParticleType::Fluid:
        return 1 / density;

    case ParticleType::Wall:
    case ParticleType::DummyWall:
    default:
        return 0;
    }
}
