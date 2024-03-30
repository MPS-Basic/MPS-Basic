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
    this->ppe                      = PressurePoissonEquation(
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
    std::vector<int> ignoreIds;
    for (auto& p : particles) {
        if (p.boundaryCondition != FluidState::Inner) {
            ignoreIds.push_back(p.id);
        }
    }

    std::sort(ignoreIds.begin(), ignoreIds.end());
    this->ppe.make(particles, ignoreIds);
    this->pressure = this->ppe.solve();
    removeNegativePressure();

    return this->pressure;
}

Implicit::~Implicit() {
}

// void Implicit::applyBoundaryCondition(const std::vector<Particle>& particles) {
//     std::vector<int> NotInnerParticleIds;
//     for (size_t i = 0; i < particles.size(); i++) {
//         if (particles[i].boundaryCondition != FluidState::Inner) {
//             NotInnerParticleIds.push_back(i);
//         }
//     }

//     this->ppe.removeParticlesFromCalculation(NotInnerParticleIds);
// }

// this function is not used.
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
