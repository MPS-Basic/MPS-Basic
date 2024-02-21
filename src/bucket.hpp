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
	int num, numX, numY;
	double length;
	std::vector<int> next, first, last;

	void generate(const int& particleNum);
	void set(const double& reMax, const double& CFL, const Domain& domain, const size_t& particleSize);
	/**
	 * @brief store particles in the bucket
	 * @param particles partiles to be stored
	 * @param domain domain of the simulation
	 */
	void storeParticles(std::vector<Particle>& particles, const Domain& domain);
};
