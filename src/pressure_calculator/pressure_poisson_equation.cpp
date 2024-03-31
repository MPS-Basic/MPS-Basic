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

void PressurePoissonEquation::make(const std::vector<Particle>& particles, const std::vector<int>& excludedIds) {
    this->particlesCount = particles.size();
    std::sort(excludedIds.begin(), excludedIds.end());
    resetEquation();
    setSourceTerm(particles, excludedIds);
    setMatrixTriplets(particles, excludedIds);
}

void PressurePoissonEquation::resetEquation() {
    coefficientMatrix.resize(particlesCount, particlesCount);
    sourceTerm.resize(particlesCount);
    matrixTriplets.clear();
}

std::vector<double> PressurePoissonEquation::solve() {
    using std::cerr;
    using std::endl;

    coefficientMatrix.setFromTriplets(matrixTriplets.begin(), matrixTriplets.end());
    Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> solver;
    solver.compute(coefficientMatrix);
    Eigen::VectorXd pressure = solver.solve(sourceTerm);
    if (solver.info() != Eigen::Success) {
        cerr << "Pressure calculation failed." << endl;
        std::exit(-1);
    }

    // this->pressure is defined as Eigen::VectorXd to solve pressure Poisson equation
    // using the BiGCSTAB method in Eigen,
    // but it is converted to std::vector<double> to return the result.
    // This conversion is done by giving std::vector
    // the pointers to the first and the last elements of the Eigen::VectorXd.
    // If this type of conversion appears frequently,
    // consider defining a function to convert the vector type.
    std::vector<double> pressureStdVec(pressure.data(), pressure.data() + pressure.size());
    return pressureStdVec;
}

void PressurePoissonEquation::setSourceTerm(
    const std::vector<Particle>& particles, const std::vector<int>& excludedIds
) {
    double n0    = this->n0_forNumberDensity;
    double gamma = this->relaxationCoefficient;

#pragma omp parallel for
    for (auto& pi : particles) {
        if (std::binary_search(excludedIds.begin(), excludedIds.end(), pi.id)) {
            sourceTerm[pi.id] = 0.0;
        } else {
            sourceTerm[pi.id] = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
        }
    }
}

void PressurePoissonEquation::setMatrixTriplets(
    const std::vector<Particle>& particles, const std::vector<int>& excludedIds
) {
    auto a  = 2.0 * dimension / (n0_forLaplacian * lambda0);
    auto re = reForLaplacian;

    for (auto& pi : particles) {
        if (std::binary_search(excludedIds.begin(), excludedIds.end(), pi.id)) {
            continue;
        }

        double coefficient_ii = 0.0;
        for (auto& neighbor : pi.neighbors) {
            auto& pj = particles[neighbor.id];
            if (pj.boundaryCondition == FluidState::Ignored) {
                continue;
            }

            if (neighbor.distance < re) {
                double coefficient_ij = a * weight(neighbor.distance, re) / fluidDensity;
                matrixTriplets.emplace_back(pi.id, pj.id, -1.0 * coefficient_ij);
                coefficient_ii += coefficient_ij;
            }
        }
        coefficient_ii += (compressibility) / (dt * dt);
        matrixTriplets.emplace_back(pi.id, pi.id, coefficient_ii);
    }
}
