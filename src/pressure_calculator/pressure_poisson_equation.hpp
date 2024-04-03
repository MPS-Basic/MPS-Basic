#pragma once

#include "../particles.hpp"
#include "target_id_map.hpp"

#include <Eigen/Sparse>
#include <vector>

namespace PressureCalculator {

/**
 * @brief Class for setting up and solving pressure Poisson equation
 */
class PressurePoissonEquation {
public:
    PressurePoissonEquation() = default;

    PressurePoissonEquation(
        int dimension,
        double dt,
        double relaxationCoefficient,
        double fluidDensity,
        double compressibility,
        double n0_forNumberDensity,
        double n0_forLaplacian,
        double lambda0,
        double reForLaplacian,
        double reForNumberDensity
    );

    /**
     * @brief Setup pressure Poisson equation
     * @param particles Particles
     * @param excludedIds Ids of particles to exclude from the pressure update. Default is empty. These particles do
     * interact as neighboring particles in the pressure update of other particles.
     */
    void setup(const Particles& particles, const TargetIdMap& targetIdMap);

    /**
     * @brief Solve pressure Poisson equation
     * @return Calculated pressure. The size of the vector is the same as the number of particles.
     */
    std::vector<double> solve();

private:
    int dimension;
    double dt;
    double relaxationCoefficient;
    double fluidDensity;
    double compressibility;
    double n0_forNumberDensity;
    double n0_forLaplacian;
    double lambda0;
    double reForLaplacian;
    double reForNumberDensity;
    size_t particlesCount;

    std::vector<Eigen::Triplet<double>> matrixTriplets; ///< Triplets for coefficient matrix
    Eigen::SparseMatrix<double, Eigen::RowMajor>
        coefficientMatrix;      ///< Coefficient matrix for pressure Poisson equation
    Eigen::VectorXd sourceTerm; ///< Source term for pressure Poisson equation

    void resetEquation();
    void setSourceTerm(const Particles& particles, const TargetIdMap& targetIdMap);
    void setMatrixTriplets(const Particles& particles, const TargetIdMap& targetIdMap);
};

} // namespace PressureCalculator
