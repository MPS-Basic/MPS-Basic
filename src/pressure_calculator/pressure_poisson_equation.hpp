#pragma once

#include "../particles.hpp"
#include "dirichlet_boundary_condition.hpp"

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
     * @param isPressureUpdateTarget Function that gets a particle and returns true if the particle is a target for
     * pressure update
     */
    void setup(const Particles& particles, const DirichletBoundaryCondition& dirichletBoundaryCondition);

    /**
     * @brief Solve pressure Poisson equation
     * @return Calculated pressure. The size of the vector is the same as the number of particles.
     */
    std::vector<double> solve();

private:
    int dimension;
    double dt;
    double relaxationCoefficient;
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
    void setSourceTerm(const Particles& particles, const DirichletBoundaryCondition& dirichletBoundaryCondition);
    void setMatrixTriplets(const Particles& particles, const DirichletBoundaryCondition& dirichletBoundaryCondition);
};

} // namespace PressureCalculator
