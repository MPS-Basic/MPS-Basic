#pragma once

#include "common.hpp"
#include "weight.hpp"

/**
 *  @brief Struct for reference values of MPS method
 */
class RefValues {
public:
    double n0     = 0; ///< reference value of number density for source term of pressure Poisson equation
    double lambda = 0; ///< coefficient for laplacian

    RefValues() = default;
    /**
     * @brief calculate reference values for MPS method
     *
     * @param dim dimension of the simulation
     * @param particleDistance initial particle distance
     * @param re effective radius
     * @param weightType weight function type
     */
    RefValues(int dim, double particleDistance, double re, WeightType weightType);

private:
    Weight weight; ///< weight function instance
};
