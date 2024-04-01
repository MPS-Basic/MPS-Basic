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

int Particles::size() {
    return particles.size();
}

int Particles::size() const {
    return particles.size();
}

void Particles::add(const Particle& particle) {
    particles.emplace_back(particle);
}

Particle& Particles::operator[](size_t index) {
    return particles[index];
}
