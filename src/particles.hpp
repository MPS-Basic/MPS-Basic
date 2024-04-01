#pragma once

#include "particle.hpp"

#include <vector>

class Particles {
public:
    std::vector<Particle>::iterator begin() {
        return particles.begin();
    }

    std::vector<Particle>::const_iterator begin() const {
        return particles.begin();
    }

    std::vector<Particle>::iterator end() {
        return particles.end();
    }

    std::vector<Particle>::const_iterator end() const {
        return particles.end();
    }

private:
    std::vector<Particle> particles;
};
