#include "particles.hpp"

#include <vector>

std::vector<Particle>::iterator Particles::begin() {
    return particles.begin();
}

std::vector<Particle>::const_iterator Particles::begin() const {
    return particles.begin();
}

std::vector<Particle>::iterator Particles::end() {
    return particles.end();
}

std::vector<Particle>::const_iterator Particles::end() const {
    return particles.end();
}

int Particles::size() const {
    return particles.size();
}

void Particles::add(const Particle& particle) {
    // This class assumes that the index of the inner vector is equal to the id of the particle located there.
    // To ensure this, assert that the id of the particle is equal to the size of the inner vector.
    // assert(particle.id == particles.size());
    particles.emplace_back(particle);
}

std::vector<Particle>::iterator Particles::erase(std::vector<Particle>::iterator it) {
    return particles.erase(it);
}

Particle& Particles::operator[](size_t index) {
    return particles[index];
}

const Particle& Particles::operator[](size_t index) const {
    return particles[index];
}

Particles Particles::getNeighbors(const Particle& particle) const {
    Particles neighbors;
    for (const auto& neighbor : particle.neighbors) {
        neighbors.add(particles[neighbor.id]);
    }
    return neighbors;
}

// TODO: A better way may exist.
Eigen::Vector3d Particles::center() const {
    Eigen::Vector3d center = Eigen::Vector3d::Zero();
    for (const auto& p : particles) {
        center += p.position;
    }
    center /= particles.size();
    return center;
}
