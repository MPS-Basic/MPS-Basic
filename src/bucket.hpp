#pragma once

#include <iostream>
#include <vector>

#include "common.hpp"
#include "domain.hpp"
#include "particle.hpp"

///@brief Class for bucket for neighbor search
///@details This class is used for neighbor search in particle method.
/// In particle method, neighbor search is required for calculating interaction between particles.
/// Each particle is stored in the bucket, bucket is used for searching neighbor particles.
class Bucket {
private:
public:
	int num, numX, numY;
	double length;
	std::vector<int> next, first, last;

	void generate(const int& particleNum) {
		next.resize(particleNum);
	}

	void set(const double& reMax, const double& CFL, const Domain& domain, const size_t& particleSize) {
		length = reMax * (1.0 + CFL);

		numX     = (int) (domain.xLength / length) + 3;
		numY     = (int) (domain.yLength / length) + 3;
		int numZ = (int) (domain.zLength / length) + 3;
		num      = numX * numY * numZ;

		first.resize(num);
		last.resize(num);
		next.resize(particleSize);
	}

	/// @brief store particles in the bucket
	/// @param particles partiles to be stored
	/// @param domain domain of the simulation
	void storeParticles(std::vector<Particle>& particles, const Domain& domain) {
#pragma omp parallel for
		rep(i, 0, num) {
			first[i] = -1;
			last[i]  = -1;
		}
#pragma omp parallel for
		rep(i, 0, particles.size()) {
			next[i] = -1;
		}

		for (auto& p : particles) {
			if (p.type == ParticleType::Ghost)
				continue;

			bool isInDomain = true;
			if (p.position.x() < domain.xMin || domain.xMax < p.position.x())
				isInDomain = false;
			if (p.position.y() < domain.yMin || domain.yMax < p.position.y())
				isInDomain = false;
			if (p.position.z() < domain.zMin || domain.zMax < p.position.z())
				isInDomain = false;
			if (!isInDomain) {
				std::cerr << "WARNING: particle " << p.id << " is out of domain." << std::endl;
				std::cerr << "x = " << p.position.x() << " ";
				std::cerr << "y = " << p.position.y() << " ";
				std::cerr << "z = " << p.position.z() << std::endl;
				p.type = ParticleType::Ghost;
				continue;
				// std::exit(-1);
			}

			int ix      = (int) ((p.position.x() - domain.xMin) / length) + 1;
			int iy      = (int) ((p.position.y() - domain.yMin) / length) + 1;
			int iz      = (int) ((p.position.z() - domain.zMin) / length) + 1;
			int iBucket = ix + iy * numX + iz * numX * numY;

			if (last[iBucket] == -1)
				first[iBucket] = p.id;
			else
				next[last[iBucket]] = p.id;
			last[iBucket] = p.id;
		}
	}
};
