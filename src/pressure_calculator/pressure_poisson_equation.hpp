#pragma once

#include "../particle.hpp"

#include <Eigen/Sparse>
#include <vector>

namespace PressureCalculator {

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

    void make(const std::vector<Particle>& particles, const std::vector<int>& excludedIds = {});
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

    std::vector<Eigen::Triplet<double>> matrixTriplets; ///< Triplet for coefficient matrix
    Eigen::SparseMatrix<double, Eigen::RowMajor>
        coefficientMatrix;      ///< Coefficient matrix for pressure Poisson equation
    Eigen::VectorXd sourceTerm; ///< Source term for pressure Poisson equation

    void resetEquation();
    void setSourceTerm(const std::vector<Particle>& particles, const std::vector<int>& excludedIds = {});
    void setMatrixTriplets(const std::vector<Particle>& particles, const std::vector<int>& excludedIds = {});
    // void zeroOutMatrixRow(int row);
    // void zeroOutMatrixColumn(int column);
    // void zeroOutSourceTerm(int index);
};

} // namespace PressureCalculator
