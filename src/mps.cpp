#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "bucket.hpp"
#include "common.hpp"
#include "domain.hpp"
#include "loader.hpp"
#include "output.hpp"
#include "particle.hpp"
#include "refvalues.hpp"
#include "settings.hpp"
#include "simulation.hpp"
#include "system.hpp"
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

class MPS {
public:
	Settings settings;
	RefValues refValues;
	vector<Particle> particles;
	Bucket bucket;
	Domain domain;

	// pressure Poisson equation
	SparseMatrix<double, Eigen::RowMajor> coefficientMatrix;
	VectorXd sourceTerm;
	VectorXd pressure;

	// simulation parameters
	double initialTime, Time;
	int timestep   = 0;
	int fileNumber = 0;
	clock_t simStartTime, timestepStartTime, timestepEndTime;
	double courant;
	FILE* logFile;

	MPS(const Settings& settings, const vector<Particle>& particles, double initialTime) {
		this->settings    = settings;
		this->domain      = settings.domain;
		this->particles   = particles;
		this->initialTime = initialTime;
		this->Time        = initialTime;
	}

	void run() {
		startSimulation(simStartTime);

		// omp_set_num_threads(settings.numPhysicalCores);
		refValues.calc(settings.dim, settings.particleDistance, settings.re_forNumberDensity, settings.re_forGradient,
		               settings.re_forLaplacian);
		bucket.set(settings.reMax, settings.cflCondition, domain, particles.size());
		writeData();

		while (Time < settings.finishTime) {
			timestepStartTime = std::clock();

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

			timestep++;
			Time += settings.dt;
			timestepEndTime = std::clock();
			writeData();
		}

		endSimulation(simStartTime, logFile);
	};

private:
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

	void calPressure() {
		calNumberDensity(settings.re_forNumberDensity);
		setBoundaryCondition();
		setSourceTerm();
		setMatrix(settings.re_forLaplacian);
		solveSimultaneousEquations();
		removeNegativePressure();
		setMinimumPressure(settings.re_forGradient);
	}

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

	void removeNegativePressure() {
#pragma omp parallel for
		for (auto& p : particles) {
			if (p.pressure < 0.0) {
				p.pressure = 0.0;
			}
		}
	}

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

	void writeData() {
		int second, minute, hour;

		char elapsed[256];
		second = (double) (timestepEndTime - simStartTime) / CLOCKS_PER_SEC;
		calSecondMinuteHour(second, minute, hour);
		sprintf(elapsed, "elapsed=%dh %02dm %02ds", hour, minute, second);

		double ave;
		if (timestep == 0) {
			ave = 0.0;
		} else {
			ave = ((double) (timestepEndTime - simStartTime) / CLOCKS_PER_SEC) / timestep;
		}

		char remain[256];
		second = ((settings.finishTime - Time) / Time) * ave * timestep;
		calSecondMinuteHour(second, minute, hour);
		if (timestep == 0)
			sprintf(remain, "remain=-h --m --s");
		else
			sprintf(remain, "remain=%dh %02dm %02ds", hour, minute, second);

		double last = (double) (timestepEndTime - timestepStartTime) / CLOCKS_PER_SEC;

		// terminal output
		printf("%d: dt=%.gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
		       "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
		       timestep, settings.dt, Time, settings.finishTime, elapsed, remain, ave, last, fileNumber, courant);

		// log output
		// fprintf(logFile,
		//         "%d: dt=%gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
		//         "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
		//         timestep, settings.dt, Time, settings.finishTime, elapsed, remain, ave, last, fileNumber, courant);

		// error output
		fprintf(stderr, "%4d: t=%.3lfs\n", timestep, Time);

		if (Time - initialTime >= settings.outputPeriod * double(fileNumber)) {
			std::stringstream ss;
			ss << "result/prof/output_";
			ss << std::setfill('0') << std::setw(4) << fileNumber << ".prof";
			writeProf(ss, Time, particles);

			ss.str("");
			ss << "result/vtu/output_";
			ss << std::setfill('0') << std::setw(4) << fileNumber << ".vtu";
			writeVtu(ss, Time, particles);

			fileNumber++;
		}
	}

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
