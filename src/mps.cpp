#include "mps.hpp"
#include "particle.hpp"
#include "weight.hpp"
#include <queue>

using std::cerr;
using std::endl;

MPS::MPS(const Input& input) {
	this->settings  = input.settings;
	this->domain    = input.settings.domain;
	this->particles = input.particles;
}

void MPS::init() {
	refValues.calc(settings.dim, settings.particleDistance, settings.re_forNumberDensity, settings.re_forGradient,
	               settings.re_forLaplacian);
	bucket.set(settings.reMax, settings.cflCondition, domain, particles.size());
}

void MPS::stepForward() {
	bucket.storeParticles(particles, domain);
	setNeighbors(settings.reMax);
	calGravity();
	calViscosity(settings.re_forLaplacian);
	moveParticle();

	bucket.storeParticles(particles, domain);
	setNeighbors(settings.reMax);
	collision();

	bucket.storeParticles(particles, domain);
	setNeighbors(settings.reMax);
	calPressure();
	calPressureGradient(settings.re_forGradient);
	moveParticleUsingPressureGradient();

	calCourant();
}

void MPS::calGravity() {
#pragma omp parallel for
	for (auto& p : particles) {
		if (p.type == ParticleType::Fluid) {
			p.acceleration += settings.gravity;

		} else {
			p.acceleration.setZero();
		}
	}
}

void MPS::calViscosity(const double& re) {
	double n0     = refValues.n0_forLaplacian;
	double lambda = refValues.lambda;
	double a      = (settings.kinematicViscosity) * (2.0 * settings.dim) / (n0 * lambda);

#pragma omp parallel for
	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		Eigen::Vector3d viscosityTerm = Eigen::Vector3d::Zero();

		for (auto& neighbor : pi.neighbors) {
			auto& pj = particles[neighbor.id];

			if (neighbor.distance < settings.re_forLaplacian) {
				double w = weight(neighbor.distance, re);
				viscosityTerm += (pj.velocity - pi.velocity) * w;
			}
		}

		viscosityTerm *= a;
		pi.acceleration += viscosityTerm;
	}
}

void MPS::moveParticle() {
#pragma omp parallel for
	for (auto& p : particles) {
		if (p.type == ParticleType::Fluid) {
			p.velocity += p.acceleration * settings.dt;
			p.position += p.velocity * settings.dt;

		} else {
			p.acceleration.setZero();
		}
	}
}

void MPS::collision() {
	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		for (auto& neighbor : pi.neighbors) {
			Particle& pj = particles[neighbor.id];
			if (pj.type == ParticleType::Fluid && pj.id >= pi.id)
				continue;

			if (neighbor.distance < settings.collisionDistance) {

				double invMi = pi.inverseDensity(settings.fluidDensity);
				double invMj = pj.inverseDensity(settings.fluidDensity);
				double mass  = 1.0 / (invMi + invMj);

				Eigen::Vector3d normal = (pj.position - pi.position).normalized();
				double relVel          = (pj.velocity - pi.velocity).dot(normal);
				double impulse         = 0.0;
				if (relVel < 0.0)
					impulse = -(1 + settings.coefficientOfRestitution) * relVel * mass;
				pi.velocity -= impulse * invMi * normal;
				pj.velocity += impulse * invMj * normal;

				double depth           = settings.collisionDistance - neighbor.distance;
				double positionImpulse = depth * mass;
				pi.position -= positionImpulse * invMi * normal;
				pj.position += positionImpulse * invMj * normal;

				// cerr << "WARNING: Collision between particles " << pi.id << " and " << pj.id << " occurred."
				// << endl;
			}
		}
	}
}
void MPS::calNumberDensity(const double& re) {
#pragma omp parallel for
	for (auto& pi : particles) {
		pi.numberDensity = 0.0;

		if (pi.type == ParticleType::Ghost)
			continue;

		for (auto& neighbor : pi.neighbors)
			pi.numberDensity += weight(neighbor.distance, re);
	}
}
void MPS::setBoundaryCondition() {
	double n0   = refValues.n0_forNumberDensity;
	double beta = settings.surfaceDetectionRatio;

#pragma omp parallel for
	for (auto& pi : particles) {
		if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall) {
			pi.boundaryCondition = FluidState::Ignored;

		} else if (pi.numberDensity < beta * n0) {
			pi.boundaryCondition = FluidState::FreeSurface;

		} else {
			pi.boundaryCondition = FluidState::Inner;
		}
	}
}

void MPS::setSourceTerm() {
	double n0    = refValues.n0_forNumberDensity;
	double gamma = settings.relaxationCoefficientForPressure;

#pragma omp parallel for
	for (auto& pi : particles) {
		pi.sourceTerm = 0.0;
		if (pi.boundaryCondition == FluidState::Inner) {
			pi.sourceTerm = gamma * (1.0 / (settings.dt * settings.dt)) * ((pi.numberDensity - n0) / n0);
		}
	}
}

void MPS::setMatrix(const double& re) {
	std::vector<Eigen::Triplet<double>> triplets;
	auto n0 = refValues.n0_forLaplacian;
	auto a  = 2.0 * settings.dim / (n0 * refValues.lambda);
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
				double coefficient_ij = a * weight(neighbor.distance, re) / settings.fluidDensity;
				triplets.emplace_back(pi.id, pj.id, -1.0 * coefficient_ij);
				coefficient_ii += coefficient_ij;
			}
		}
		coefficient_ii += (settings.compressibility) / (settings.dt * settings.dt);
		triplets.emplace_back(pi.id, pi.id, coefficient_ii);
	}
	coefficientMatrix.setFromTriplets(triplets.begin(), triplets.end());

	// exceptionalProcessingForBoundaryCondition();
}
void MPS::exceptionalProcessingForBoundaryCondition() {
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

void MPS::solveSimultaneousEquations() {
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
void MPS::removeNegativePressure() {
#pragma omp parallel for
	for (auto& p : particles) {
		if (p.pressure < 0.0) {
			p.pressure = 0.0;
		}
	}
}
/**
 * @brief set minimum pressure for pressure gradient calculation
 * @param re effective radius \f$r_e\f$
 */
void MPS::setMinimumPressure(const double& re) {
#pragma omp parallel for
	for (auto& p : particles) {
		p.minimumPressure = p.pressure;
	}

	for (auto& pi : particles) {
		if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall)
			continue;

		for (auto& neighbor : pi.neighbors) {
			Particle& pj = particles[neighbor.id];
			if (pj.type == ParticleType::Ghost || pj.type == ParticleType::DummyWall)
				continue;
			if (pj.id > pi.id)
				continue;

			if (neighbor.distance < re) {
				pi.minimumPressure = std::min(pi.minimumPressure, pj.pressure);
				pj.minimumPressure = std::min(pj.minimumPressure, pi.pressure);
			}
		}
	}
}

void MPS::calPressureGradient(const double& re) {
	double a = settings.dim / refValues.n0_forGradient;

#pragma omp parallel for
	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		Eigen::Vector3d grad = Eigen::Vector3d::Zero();
		for (auto& neighbor : pi.neighbors) {
			Particle& pj = particles[neighbor.id];
			if (pj.type == ParticleType::Ghost || pj.type == ParticleType::DummyWall)
				continue;

			if (neighbor.distance < re) {
				double w = weight(neighbor.distance, re);
				// double dist2 = pow(neighbor.distance, 2);
				double dist2 = (pj.position - pi.position).squaredNorm();
				double pij   = (pj.pressure - pi.minimumPressure) / dist2;
				grad += (pj.position - pi.position) * pij * w;
			}
		}
		grad *= a;
		pi.acceleration -= grad * pi.inverseDensity(settings.fluidDensity);
	}
}

void MPS::moveParticleUsingPressureGradient() {
#pragma omp parallel for
	for (auto&& p : particles) {
		if (p.type == ParticleType::Fluid) {
			p.velocity += p.acceleration * settings.dt;
			p.position += p.acceleration * settings.dt * settings.dt;
		}

		p.acceleration.setZero();
	}
}

void MPS::calCourant() {
	courant = 0.0;

	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		double iCourant = (pi.velocity.norm() * settings.dt) / settings.particleDistance;
		courant         = std::max(courant, iCourant);
	}

	if (courant > settings.cflCondition) {
		cerr << "ERROR: Courant number is larger than CFL condition. Courant = " << courant << endl;
	}
}

void MPS::setNeighbors(const double& re) {
#pragma omp parallel for
	for (auto& pi : particles) {
		if (pi.type == ParticleType::Ghost)
			continue;

		pi.neighbors.clear();

		int ix = (int) ((pi.position.x() - domain.xMin) / bucket.length) + 1;
		int iy = (int) ((pi.position.y() - domain.yMin) / bucket.length) + 1;
		int iz = (int) ((pi.position.z() - domain.zMin) / bucket.length) + 1;

		for (int jx = ix - 1; jx <= ix + 1; jx++) {
			for (int jy = iy - 1; jy <= iy + 1; jy++) {
				for (int jz = iz - 1; jz <= iz + 1; jz++) {
					int jBucket = jx + jy * bucket.numX + jz * bucket.numX * bucket.numY;
					int j       = bucket.first[jBucket];

					while (j != -1) {
						Particle& pj = particles[j];

						double dist = (pj.position - pi.position).norm();
						if (j != pi.id && dist < re) {
							pi.neighbors.emplace_back(j, dist);
						}

						j = bucket.next[j];
					}
				}
			}
		}
	}
}