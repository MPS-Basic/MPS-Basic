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

void PressurePoissonEquation::make(const std::vector<Particle>& particles) {
    setSourceTerm(particles);
    setMatrix(particles);
}

// 指定した粒子を計算から除外する
void PressurePoissonEquation::removeParticleFromCalculation(int index) {
    zeroOutMatrixRow(index);
    zeroOutMatrixColumn(index);
    zeroOutSourceTerm(index);
}

// coefficientMatrix の指定された行を0にする
void PressurePoissonEquation::zeroOutMatrixRow(int row) {
    for (int i = 0; i < coefficientMatrix.outerSize(); ++i) {
        for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(coefficientMatrix, i); it; ++it) {
            if (it.row() == row) {
                it.valueRef() = 0.0;
            }
        }
    }
}

// coefficientMatrix の指定された列を0にする
void PressurePoissonEquation::zeroOutMatrixColumn(int column) {
    for (int i = 0; i < coefficientMatrix.outerSize(); ++i) {
        for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(coefficientMatrix, i); it; ++it) {
            if (it.col() == column) {
                it.valueRef() = 0.0;
            }
        }
    }
}

// sourceTerm の指定された要素を0にする
void PressurePoissonEquation::zeroOutSourceTerm(int index) {
    sourceTerm[index] = 0.0;
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

void PressurePoissonEquation::setSourceTerm(const std::vector<Particle>& particles) {
    double n0    = this->n0_forNumberDensity;
    double gamma = this->relaxationCoefficient;

    sourceTerm.resize(particles.size());

#pragma omp parallel for
    for (auto& pi : particles) {
        sourceTerm[pi.id] = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
    }
}

void PressurePoissonEquation::setMatrix(const std::vector<Particle>& particles) {
    std::vector<Eigen::Triplet<double>> triplets;
    auto a  = 2.0 * dimension / (n0_forLaplacian * lambda0);
    auto re = reForLaplacian;
    coefficientMatrix.resize(particles.size(), particles.size());

    for (auto& pi : particles) {
        double coefficient_ii = 0.0;
        for (auto& neighbor : pi.neighbors) {
            auto& pj = particles[neighbor.id];
            if (pj.boundaryCondition == FluidState::Ignored) {
                continue;
            }

            if (neighbor.distance < re) {
                double coefficient_ij = a * weight(neighbor.distance, re) / fluidDensity;
                triplets.emplace_back(pi.id, pj.id, -1.0 * coefficient_ij);
                coefficient_ii += coefficient_ij;
            }
        }
        coefficient_ii += (compressibility) / (dt * dt);
        triplets.emplace_back(pi.id, pi.id, coefficient_ii);
    }
    coefficientMatrix.setFromTriplets(triplets.begin(), triplets.end());
}
