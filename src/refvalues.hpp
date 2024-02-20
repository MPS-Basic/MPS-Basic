#pragma once
#include "common.hpp"
/**
 *  @brief Struct for reference values of MPS method
 */
class RefValues {
private:
public:
	double n0_forNumberDensity; ///< reference value of number density for source term of pressure Poisson equation
	double n0_forGradient;      ///< reference value of number density for gradient
	double n0_forLaplacian;     ///< reference value of number density for laplacian
	double lambda;              ///< coefficient for laplacian

	/**
	 * @brief calculate reference values for MPS method
	 *
	 * @param dim dimension of the simulation
	 * @param particleDistance initial particle distance
	 * @param re_forNumberDensity effective radius when calculating number density
	 * @param re_forGradient effective radius when calculating gradient
	 * @param re_forLaplacian effective radius when calculating laplacian
	 */
	void
	calc(int dim, double particleDistance, double re_forNumberDensity, double re_forGradient, double re_forLaplacian);
};
