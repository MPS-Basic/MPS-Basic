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
    auto refValuesForNumberDensity = RefValues(dimension, particleDistance, reForNumberDensity);
    auto refValuesForLaplacian     = RefValues(dimension, particleDistance, reForLaplacian);
    this->pressurePoissonEquation  = PressurePoissonEquation(
        dimension,
        dt,
        relaxationCoefficient,
        fluidDensity,
        compressibility,
        refValuesForNumberDensity.n0,
        refValuesForLaplacian.n0,
        refValuesForLaplacian.lambda,
        reForLaplacian,
        reForNumberDensity
    );
}

std::vector<double> Implicit::calc(const std::vector<Particle>& particles) {
    // Pressure update is performed only for inner particles.
    std::vector<int> excludedIds;
    for (auto& p : particles) {
        if (p.boundaryCondition != FluidState::Inner) {
            excludedIds.push_back(p.id);
        }
    }

    this->pressurePoissonEquation.make(particles, excludedIds);
    this->pressure = this->pressurePoissonEquation.solve();
    removeNegativePressure();

    return this->pressure;
}

Implicit::~Implicit() {
}

// NOTE: This function is not used.
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

void Implicit::removeNegativePressure() {
#pragma omp parallel for
    for (auto& p : pressure) {
        if (p < 0) {
            p = 0;
        }
    }
}
