#pragma once

#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "bucket.hpp"
#include "common.hpp"
#include "domain.hpp"
#include "input.hpp"
#include "particle.hpp"
#include "refvalues.hpp"
#include "settings.hpp"
#include "weight.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace Eigen;

/**
 * @brief MPS simulation class
 *
 * @details Executes the MPS simulation. This class does not handle the
 * simulation process itself, but only the calculation of the MPS method.
 */
class MPS {
public:
	Settings settings;          ///< Settings for the simulation
	RefValues refValues;        ///< Reference values for the simulation (\f$n^0\f$, \f$\lambda^0\f$)
	vector<Particle> particles; ///< Particles in the simulation
	Bucket bucket;              ///< Bucket for neighbor search
	Domain domain;              ///< Domain of the simulation

	// pressure Poisson equation
	SparseMatrix<double, Eigen::RowMajor> coefficientMatrix; ///< Coefficient matrix for pressure Poisson equation
	VectorXd sourceTerm;                                     ///< Source term for pressure Poisson equation
	VectorXd pressure;                                       ///< Solution of pressure Poisson equation

	double courant; ///< Maximum courant number among all particles
	FILE* logFile;

	MPS() = default;

	MPS(const Input& input) {
		this->settings  = input.settings;
		this->domain    = input.settings.domain;
		this->particles = input.particles;
	};

	void init() {
		refValues.calc(settings.dim, settings.particleDistance, settings.re_forNumberDensity, settings.re_forGradient,
		               settings.re_forLaplacian);
		bucket.set(settings.reMax, settings.cflCondition, domain, particles.size());
	};

	void stepForward() {
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
	};

private:
	/**
	 * @brief calculate gravity term
	 */
	void calGravity() {
#pragma omp parallel for
		for (auto& p : particles) {
			if (p.type == ParticleType::Fluid) {
				p.acceleration += settings.gravity;

			} else {
				p.acceleration.setZero();
			}
		}
	}

	/**
	 * @brief calculate viscosity term of Navier-Stokes equation
	 * @param re effective radius \f$r_e\f$
	 * @details
	 * The viscosity term of the Navier-Stokes equation is calculated as follows:
	 * \f[
	 * \nu\langle \nabla^2\mathbf{u}\rangle_i = \nu\frac{2 d}{n^0\lambda^0}\sum_{j\neq i} (\mathbf{u}_j - \mathbf{u}_i)
	 * w_{ij} \f]
	 */
	void calViscosity(const double& re) {
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
	/**
	 * @brief move particles in prediction step
	 * @details
	 * The position and velocity of each particle are updated as
	 * \f[
	 * \mathbf{u}_i^* = \mathbf{u}_i^k + (\nu\langle \nabla^2\mathbf{u}\rangle^k_i+\mathbf{g})\Delta t.
	 * \f]
	 */
	void moveParticle() {
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

	/**
	 *@brief calculate collision between particles when they are too close
	 */
	void collision() {
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

					// std::cerr << "WARNING: Collision between particles " << pi.id << " and " << pj.id << " occurred."
					// << std::endl;
				}
			}
		}
	}

	/**
	 * @brief calculate pressure using pressure Poisson equation
	 */
	void calPressure() {
		calNumberDensity(settings.re_forNumberDensity);
		setBoundaryCondition();
		setSourceTerm();
		setMatrix(settings.re_forLaplacian);
		solveSimultaneousEquations();
		removeNegativePressure();
		setMinimumPressure(settings.re_forGradient);
	}

	/**
	 * @brief calculate number density of each particle
	 * @param re effective radius \f$r_e\f$
	 */
	void calNumberDensity(const double& re) {
#pragma omp parallel for
		for (auto& pi : particles) {
			pi.numberDensity = 0.0;

			if (pi.type == ParticleType::Ghost)
				continue;

			for (auto& neighbor : pi.neighbors)
				pi.numberDensity += weight(neighbor.distance, re);
		}
	}
	/**
	 *@brief set boundary condition of pressure Poisson equation
	 */
	void setBoundaryCondition() {
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

	/**
	 * @brief set source term of pressure Poisson equation
	 * @details
	 * The source term of the pressure Poisson equation is calculated as follows:
	 * \f[
	 * b_i = \frac{\gamma}{\Delta t^2}\frac{n_i-n^0}{n^0}
	 * \f]
	 */
	void setSourceTerm() {
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
	/**
	 * @brief set coefficient matrix of pressure Poisson equation
	 * @param re  effective radius \f$r_e\f$
	 * @details
	 * Applying Laplacian model to pressure Poisson equation, the coefficient matrix is calculated as follows:
	 * \f[
	 *	-\frac{1}{\rho^0}\langle\nabla^2 P\rangle_i^{k+1} = b_i.
	 * \f]
	 */
	void setMatrix(const double& re) {
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

	void exceptionalProcessingForBoundaryCondition() {
		/* If there is no Dirichlet boundary condition on the fluid,
		   increase the diagonal terms of the matrix for an exception. This
		   allows us to solve the matrix without Dirichlet boundary conditions.
		 */
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
	/**
	 * @brief Solve the pressure Poisson equation
	 * @details
	 * The pressure Poisson equation is solved using the BiCGSTAB method.
	 */
	void solveSimultaneousEquations() {
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
			std::cerr << "Pressure calculation failed." << std::endl;
			std::exit(-1);
		}

#pragma omp parallel for
		for (auto& pi : particles) {
			pi.pressure = pressure[pi.id];
		}
	}
	/**
	 * @brief remove negative pressure for stability
	 */
	void removeNegativePressure() {
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
	void setMinimumPressure(const double& re) {
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
	/**
	 * @brief calculate pressure gradient term
	 * @param re
	 * @details
	 * The pressure gradient term of the Navier-Stokes equation is calculated as
	 * \f[
	 * -\frac{1}{\rho^0}\langle\nabla P\rangle_i = -\frac{1}{\rho^0}\frac{d}{n^0}\sum_{j\neq i}
	 * \frac{P_j-P'_i}{\|\mathbf{r}_{ij}\|^2}\mathbf{r}_{ij} w_{ij} \f] where \f$P'_i\f$ is the minimum pressure of
	 * the particle \f$i\f$.
	 */
	void calPressureGradient(const double& re) {
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
	/**
	 * @brief move particles in correction step
	 * @details
	 * The position and velocity of each particle are updated as
	 * \f[
	 * \mathbf{u}_i^{k+1} = \mathbf{u}_i^* - \frac{1}{\rho^0} \langle\nabla P^{k+1} \rangle_i \Delta t.
	 * \f]
	 */
	void moveParticleUsingPressureGradient() {
#pragma omp parallel for
		for (auto&& p : particles) {
			if (p.type == ParticleType::Fluid) {
				p.velocity += p.acceleration * settings.dt;
				p.position += p.acceleration * settings.dt * settings.dt;
			}

			p.acceleration.setZero();
		}
	}
	/**
	 * @brief calculate Courant number
	 */
	void calCourant() {
		courant = 0.0;

		for (auto& pi : particles) {
			if (pi.type != ParticleType::Fluid)
				continue;

			double iCourant = (pi.velocity.norm() * settings.dt) / settings.particleDistance;
			courant         = std::max(courant, iCourant);
		}

		if (courant > settings.cflCondition) {
			std::cerr << "ERROR: Courant number is larger than CFL condition. Courant = " << courant << std::endl;
		}
	}

	/**
	 * @brief search neighbors of each particle
	 * @param re effective radius
	 */
	void setNeighbors(const double& re) {
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
};
