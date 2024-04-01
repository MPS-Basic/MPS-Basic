#pragma once

#include "particle.hpp"

#include <vector>

class Particles {
public:
    // functions to make Particles iterable
    std::vector<Particle>::iterator begin();
    std::vector<Particle>::const_iterator begin() const;
    std::vector<Particle>::iterator end();
    std::vector<Particle>::const_iterator end() const;

    int size();
    int size() const;
    void add(const Particle& particle);
    Particle& operator[](size_t index);

private:
    std::vector<Particle> particles;
};
