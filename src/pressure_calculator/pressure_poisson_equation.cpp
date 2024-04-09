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

void PressurePoissonEquation::setup(
    const Particles& particles, const DirichletBoundaryCondition& dirichletBoundaryCondition
) {
    this->particlesCount = particles.size();

    resetEquation();
    setSourceTerm(particles, dirichletBoundaryCondition);
    setMatrixTriplets(particles, dirichletBoundaryCondition);
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
 * @param isPressureUpdateTarget Function that gets a particle and returns true if the particle is a target for pressure
 * update
 */
void PressurePoissonEquation::setSourceTerm(
    const Particles& particles, const DirichletBoundaryCondition& dirichletBoundaryCondition
) {
    double n0    = this->n0_forNumberDensity;
    double gamma = this->relaxationCoefficient;

#pragma omp parallel for
    for (auto& pi : particles) {
        if (dirichletBoundaryCondition.contains(pi.id)) {
            // When dirichlet boundary condition is set, the source term is the boundary condition value.
            sourceTerm[pi.id] = dirichletBoundaryCondition.value(pi.id);
        } else {
            sourceTerm[pi.id] = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
        }
    }
}

/**
 * @brief Set the matrix triplets for the pressure Poisson equation
 * @details Coefficient matrix is a sparse matrix and made by triplets. Triplets are the elements of the matrix that
 * are not zero and represented by the row index, column index, and the value of the element. This function sets the
 * triplets for the pressure Poisson equation.
 * @param particles Particles
 * @param isPressureUpdateTarget Function that gets a particle and returns true if the particle is a target for pressure
 * update
 */
void PressurePoissonEquation::setMatrixTriplets(
    const Particles& particles, const DirichletBoundaryCondition& dirichletBoundaryCondition
) {
    auto a  = 2.0 * dimension / (n0_forLaplacian * lambda0);
    auto re = reForLaplacian;

    for (auto& pi : particles) {
        if (dirichletBoundaryCondition.contains(pi.id)) {
            // When Dirichlet boundary conditions are set, only the diagonal term is set to 1 so that the pressure is at
            // the specified value.
            matrixTriplets.emplace_back(pi.id, pi.id, 1.0);
            continue;
        }

        double coefficient_ii = 0.0;
        for (auto& neighbor : pi.neighbors) {
            auto& pj = particles[neighbor.id];
            if (pj.type == ParticleType::Ghost || pj.type == ParticleType::DummyWall) {
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
