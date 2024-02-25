#include "particle.hpp"
Particle::Particle(int id, ParticleType type, Eigen::Vector3d pos, Eigen::Vector3d vel) {
    this->id       = id;
    this->type     = type;
    this->position = pos;
    this->velocity = vel;
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