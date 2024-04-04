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
    assert(particle.id == particles.size());
    particles.emplace_back(particle);
}

Particle& Particles::operator[](size_t index) {
    return particles[index];
}

const Particle& Particles::operator[](size_t index) const {
    return particles[index];
}
