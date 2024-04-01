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
