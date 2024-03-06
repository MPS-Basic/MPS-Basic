#pragma once

#include "common.hpp"
#include "domain.hpp"
#include "particle.hpp"

#include <iostream>
#include <vector>

/**
 * @brief Class for bucket for neighbor search
 *
 * @details This class is used for neighbor search in particle method.
 * In particle method, neighbor search is required for calculating interaction between particles.
 * Each particle is stored in the bucket, bucket is used for searching neighbor particles.
 */
class Bucket {
private:
public:
    int num{}, numX{}, numY{}, numZ{};
    double length{};
    Domain domain{};
    std::vector<int> next, first, last;

    /**
     * @brief Change the length of "next" array to match the number of particles in the domain
     * @param particleNum Number of fluid particles in the domain
     */
    void generate(const int& particleNum);

    /**
     * @brief Divide domain into lattice segments
     * @param reMax Maximum of effective radius
     * @param CFL  CFL Condition
     * @param domain domain of the simulation
     * @param particleSize Number of fluid particles in the domain
     */
    void set(const double& reMax, const double& CFL, const Domain& domain, const size_t& particleSize);

    /**
     * @brief Store particles in the bucket
    Bucket() = default;

    Bucket(const double& reMax, const Domain& domain, const size_t& particleSize);
    void generate(const int& particleNum);
    /**
     * @brief store particles in the bucket
     * @param particles particles to be stored
     * @param domain domain of the simulation
     */
    void storeParticles(std::vector<Particle>& particles);
};
