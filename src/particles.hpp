#pragma once

#include "particle.hpp"

#include <vector>

/**
 * @brief A collection of particles
 */
class Particles {
public:
    // functions to make Particles iterable
    std::vector<Particle>::iterator begin();
    std::vector<Particle>::const_iterator begin() const;
    std::vector<Particle>::iterator end();
    std::vector<Particle>::const_iterator end() const;

    /**
     * @brief Get the number of particles
     *
     * @return int the number of particles
     */
    int size() const;

    /**
     * @brief Add a particle to the collection
     *
     * @param particle the particle to add
     */
    void add(const Particle& particle);

    /**
     * @brief Get a particle by index
     *
     * @param index the index of the particle
     * @return Particle& the particle
     */
    Particle& operator[](size_t index);

private:
    std::vector<Particle> particles;
};
