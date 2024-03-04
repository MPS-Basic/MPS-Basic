#include "pressure_poisson_equation.hpp"

#include "weight.hpp"

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

void make(const std::vector<Particle>& particles) {
    setSourceTerm(particles);
    setMatrix(particles);
}

std::vector<double> solve() {
    using std::cerr;
    using std::endl;

    Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> solver;
    solver.compute(coefficientMatrix);
    auto pressure = solver.solve(sourceTerm);
    if (solver.info() != Eigen::Success) {
        cerr << "Pressure calculation failed." << endl;
        std::exit(-1);
    }

    std::vector<double> pressureStdVec(pressure.data(), pressure.data() + pressure.size());
    return pressureStdVec;
}

void setSourceTerm(const std::vector<particles>& particles) {
    double n0    = this->n0_forNumberDensity;
    double gamma = this->relaxationCoefficient;

    sourceTerm.resize(particles.size());

#pragma omp parallel for
    for (auto& pi : particles) {
        sourceTerm[pi.id] = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
    }
}

void setMatrix(const std::vector<particles>& particles) {
    std::vector<Eigen::Triplet<double>> triplets;
    auto a  = 2.0 * dimension / (n0_forLaplacian * lambda0);
    auto re = reForLaplacian;
    coefficientMatrix.resize(particles.size(), particles.size());

    for (auto& pi : particles) {
        double coefficient_ii = 0.0;
        for (auto& neighbor : pi.neighbors) {
            Particle& pj = particles[neighbor.id];

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
