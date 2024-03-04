#include "implicit.hpp"

#include "../refvalues.hpp"
#include "../weight.hpp"

#include <iostream>
#include <queue>

using PressureCalculator::Implicit;
using std::cerr;
using std::endl;

Implicit::Implicit(
    int dimension,
    double particleDistance,
    double reForNumberDensity,
    double reForLaplacian,
    double dt,
    double fluidDensity,
    double compressibility,
    double relaxationCoefficient
) {
    this->dimension                 = dimension;
    this->reForNumberDensity        = reForNumberDensity;
    this->reForLaplacian            = reForLaplacian;
    this->dt                        = dt;
    this->fluidDensity              = fluidDensity;
    this->compressibility           = compressibility;
    this->relaxationCoefficient     = relaxationCoefficient;
    this->refValuesForNumberDensity = RefValues(dimension, particleDistance, reForNumberDensity);
    this->refValuesForLaplacian     = RefValues(dimension, particleDistance, reForLaplacian);
}

std::vector<double> Implicit::calc(const std::vector<Particle>& particles) {
    this->particles = particles;
    setSourceTerm();
    setMatrix();
    solveSimultaneousEquations();
    removeNegativePressure();

    // this->pressure is defined as Eigen::VectorXd to solve pressure Poisson equation
    // using the BiGCSTAB method in Eigen,
    // but it is converted to std::vector<double> to return the result.
    // This conversion is done by giving std::vector
    // the pointers to the first and the last elements of the Eigen::VectorXd.
    // If this type of conversion appears frequently,
    // consider defining a function to convert the vector type.
    std::vector<double> pressureStdVec(this->pressure.data(), this->pressure.data() + this->pressure.size());

    return pressureStdVec;
}

Implicit::~Implicit() {
}

void Implicit::setSourceTerm() {
    double n0    = refValuesForNumberDensity.n0;
    double gamma = relaxationCoefficient;

    sourceTerm.resize(particles.size());

#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.boundaryCondition == FluidState::Inner) {
            sourceTerm[pi.id] = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
        } else {
            sourceTerm[pi.id] = 0.0;
        }
    }
}

void Implicit::setMatrix() {
    std::vector<Eigen::Triplet<double>> triplets;
    auto a  = 2.0 * dimension / (refValuesForLaplacian.n0 * refValuesForLaplacian.lambda);
    auto re = reForLaplacian;
    coefficientMatrix.resize(particles.size(), particles.size());

    for (auto& pi : particles) {
        if (pi.boundaryCondition != FluidState::Inner)
            continue;

        double coefficient_ii = 0.0;
        for (auto& neighbor : pi.neighbors) {
            Particle& pj = particles[neighbor.id];
            if (pj.boundaryCondition == FluidState::Ignored)
                continue;

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

    // exceptionalProcessingForBoundaryCondition();
}

void Implicit::exceptionalProcessingForBoundaryCondition() {
    std::vector<bool> checked(particles.size(), false);
    std::vector<bool> connected(particles.size(), false);

    for (auto& pi : particles) {
        if (pi.boundaryCondition == FluidState::FreeSurface)
            connected[pi.id] = true;
        if (connected[pi.id])
            continue;
        // BFS for connected components
        std::queue<size_t> queue;
        queue.push(pi.id);
        checked[pi.id] = true;
        while (!queue.empty()) {
            // pop front element
            auto v = queue.front();
            queue.pop();
            // search for adjacent nodes
            for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(coefficientMatrix, v); it; ++it) {
                auto nv = it.col();
                if (!checked[nv]) {
                    if (connected[nv]) { // connected to boundary
                        connected[v] = true;
                        checked[v]   = true;
                        break;

                    } else {
                        queue.push(nv);
                        checked[nv] = true;
                    }
                }
            }
        }
    }
}

void Implicit::solveSimultaneousEquations() {
    pressure.resize(particles.size());

    Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> solver;
    solver.compute(coefficientMatrix);
    pressure = solver.solve(sourceTerm);
    if (solver.info() != Eigen::Success) {
        cerr << "Pressure calculation failed." << endl;
        std::exit(-1);
    }
}

void Implicit::removeNegativePressure() {
#pragma omp parallel for
    for (auto& p : pressure) {
        if (p < 0) {
            p = 0;
        }
    }
}
