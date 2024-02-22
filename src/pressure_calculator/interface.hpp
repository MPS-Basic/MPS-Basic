#pragma once

#include "../particle.hpp"
#include <vector>

/**
 * @brief Pressure calculator interface
 *
 * @details Interface for pressure calculator. This interface is used to
 * calculate pressure in the MPS method.
 */
class IPressureCalculator {
public:
	/**
	 * @brief calculate pressure
	 * @param particles particles
	 */
	virtual void calc(std::vector<Particle>& particles) = 0;

	/**
	 * @brief destructor
	 */
	virtual ~IPressureCalculator() {};
};
