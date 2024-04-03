#include "pressure_poisson_equation.hpp"

#include "../weight.hpp"

#include <iostream>

using PressureCalculator::PressurePoissonEquation;

PressurePoissonEquation::PressurePoissonEquation(
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
) {
    this->dimension             = dimension;
    this->dt                    = dt;
    this->relaxationCoefficient = relaxationCoefficient;
    this->fluidDensity          = fluidDensity;
    this->compressibility       = compressibility;
    this->n0_forNumberDensity   = n0_forNumberDensity;
    this->n0_forLaplacian       = n0_forLaplacian;
    this->lambda0               = lambda0;
    this->reForLaplacian        = reForLaplacian;
    this->reForNumberDensity    = reForNumberDensity;
}

void PressurePoissonEquation::setup(const Particles& particles, const TargetIdMap& targetIdMap) {
    this->particlesCount = particles.size();

    resetEquation();
    setSourceTerm(particles, targetIdMap);
    setMatrixTriplets(particles, targetIdMap);
    coefficientMatrix.setFromTriplets(matrixTriplets.begin(), matrixTriplets.end());
}

std::vector<double> PressurePoissonEquation::solve() {
    using std::cerr;
    using std::endl;

    Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> solver;
    solver.compute(coefficientMatrix);
    Eigen::VectorXd pressure = solver.solve(sourceTerm);
    if (solver.info() != Eigen::Success) {
        cerr << "Pressure calculation failed." << endl;
        std::exit(-1);
    }

    // This conversion is done by giving std::vector the pointers to the first and the last elements of the
    // Eigen::VectorXd. If this type of conversion appears frequently, consider defining a function to convert the
    // vector type.
    std::vector<double> pressureStdVec(pressure.data(), pressure.data() + pressure.size());
    return pressureStdVec;
}

void PressurePoissonEquation::resetEquation() {
    coefficientMatrix.resize(particlesCount, particlesCount);
    sourceTerm.resize(particlesCount);
    matrixTriplets.clear();
}

/**
 * @brief Set the source term for the pressure Poisson equation
 * @param particles Particles
 * @param excludedIds Ids of particles to exclude from the pressure update.
 * @attention excludedIds should be sorted.
 */
void PressurePoissonEquation::setSourceTerm(const Particles& particles, const TargetIdMap& targetIdMap) {
    double n0    = this->n0_forNumberDensity;
    double gamma = this->relaxationCoefficient;

#pragma omp parallel for
    for (auto& pi : particles) {
        if (targetIdMap.has(pi.id)) {
            sourceTerm[pi.id] = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
        } else {
            sourceTerm[pi.id] = 0.0;
        }
    }
}

/**
 * @brief Set the matrix triplets for the pressure Poisson equation
 * @details Coefficient matrix is a sparse matrix and made by triplets. Triplets are the elements of the matrix that
 * are not zero and represented by the row index, column index, and the value of the element. This function sets the
 * triplets for the pressure Poisson equation.
 * @param particles Particles
 * @param excludedIds Ids of particles to exclude from the pressure update.
 * @attention excludedIds should be sorted.
 */
void PressurePoissonEquation::setMatrixTriplets(const Particles& particles, const TargetIdMap& targetIdMap) {
    auto a  = 2.0 * dimension / (n0_forLaplacian * lambda0);
    auto re = reForLaplacian;

    for (auto& targetIdPair : targetIdMap) {
        double coefficient_ii   = 0.0;
        auto i                  = targetIdPair.first;
        auto& targetNeighborIds = targetIdPair.second;
        auto& pi                = particles[i];

        for (auto& neighbor : pi.neighbors) {
            // neighbor.id が targetNeighborIds に含まれなければスキップ
            if (targetNeighborIds.find(neighbor.id) == targetNeighborIds.end()) {
                continue;
            }

            if (neighbor.distance < re) {
                double coefficient_ij = a * weight(neighbor.distance, re) / fluidDensity;
                matrixTriplets.emplace_back(i, neighbor.id, -1.0 * coefficient_ij);
                coefficient_ii += coefficient_ij;
            }
        }
        coefficient_ii += (compressibility) / (dt * dt);
        matrixTriplets.emplace_back(i, i, coefficient_ii);
    }
}
