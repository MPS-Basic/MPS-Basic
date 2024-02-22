#include "implicit.hpp"
#include <queue>

using namespace std::cerr;
using namespace std::endl;

void ImplicitPressureCalculator::calc(std::vector<Particle>& particles) {
    this->particles = particles;
    setSourceTerm();
    setMatrix();
    solveSimultaneousEquations();
    removeNegativePressure();
}

void ImplicitPressureCalculator::setSourceTerm() {
	double n0    = n0_forNumberDensity;
	double gamma = relaxationCoefficient;

#pragma omp parallel for
	for (auto& pi : particles) {
		pi.sourceTerm = 0.0;
		if (pi.boundaryCondition == FluidState::Inner) {
			pi.sourceTerm = gamma * (1.0 / (dt * dt)) * ((pi.numberDensity - n0) / n0);
		}
	}
}

void ImplicitPressureCalculator::setMatrix() {
	std::vector<Eigen::Triplet<double>> triplets;
	auto n0 = n0_forLaplacian;
	auto a  = 2.0 * dimension / (n0 * lambda);
    auto re = re_forLaplacian;
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


void ImplicitPressureCalculator::exceptionalProcessingForBoundaryCondition() {
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

void ImplicitPressureCalculator::solveSimultaneousEquations() {
	sourceTerm.resize(particles.size());
	pressure.resize(particles.size());

#pragma omp parallel for
	for (auto& pi : particles) {
		sourceTerm[pi.id] = pi.sourceTerm;
	}

	Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> solver;
	solver.compute(coefficientMatrix);
	pressure = solver.solve(sourceTerm);
	if (solver.info() != Eigen::Success) {
		cerr << "Pressure calculation failed." << endl;
		std::exit(-1);
	}

#pragma omp parallel for
	for (auto& pi : particles) {
		pi.pressure = pressure[pi.id];
	}
}

void ImplicitPressureCalculator::removeNegativePressure() {
#pragma omp parallel for
	for (auto& p : particles) {
		if (p.pressure < 0.0) {
			p.pressure = 0.0;
		}
	}
}