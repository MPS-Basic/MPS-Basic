#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "bucket.hpp"
#include "common.hpp"
#include "domain.hpp"
#include "output.hpp"
#include "particle.hpp"
#include "system.hpp"
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

// computational condition
constexpr double PARTICLE_DISTANCE     = 0.025;
constexpr double DT                    = 0.001;
constexpr double FINISH_TIME           = 2.0;
constexpr double OUTPUT_PERIOD         = 0.04;
constexpr double CFL_CONDITION         = 0.2;
constexpr int NUMBER_OF_PHYSICAL_CORES = 4;

constexpr int DIM = 2;
const Eigen::Vector3d G(0.0, -9.8, 0.0);
// constexpr int DIM = 3;
// const Eigen::Vector3d G(0.0, 0.0, -9.8);

// effective radius
constexpr double RADIUS_FOR_NUMBER_DENSITY = 2.1 * PARTICLE_DISTANCE;
constexpr double RADIUS_FOR_GRADIENT       = 2.1 * PARTICLE_DISTANCE;
constexpr double RADIUS_FOR_LAPLACIAN      = 3.1 * PARTICLE_DISTANCE;

// physical properties
constexpr double KINEMATIC_VISCOSITY = 1.0e-6;
constexpr double FLUID_DENSITY       = 1000.0;

// free surface detection
constexpr double THRESHOLD_RATIO_OF_NUMBER_DENSITY = 0.97;

// collision
constexpr double COLLISION_DISTANCE         = 0.5 * PARTICLE_DISTANCE;
constexpr double COEFFICIENT_OF_RESTITUTION = 0.2;

// parameters for pressure Poisson equation
constexpr double COMPRESSIBILITY                     = 0.45E-9;
constexpr double RELAXATION_COEFFICIENT_FOR_PRESSURE = 0.2;

void setParameters();

// main loop
void calGravity();
void calViscosity();
void moveParticle();
void collision();
void calPressure();
void calPressureGradient();
void moveParticleUsingPressureGradient();

// pressure calculation
void calNumberDensity();
void setBoundaryCondition();
void setSourceTerm();
void setMatrix();
void exceptionalProcessingForBoundaryCondition();
void solveSimultaneousEquations();
void removeNegativePressure();
void setMinimumPressure();
void calCourant();

void readData();
void writeData();

void setNeighbors();
double weight(double distance, double re);

// simulation parameters
double initialTime, Time;
int timestep   = 0;
int fileNumber = 0;
clock_t simStartTime, timestepStartTime, timestepEndTime;
double courant;
FILE* logFile;

// effective radius
double re_forNumberDensity = RADIUS_FOR_NUMBER_DENSITY;
double re_forGradient      = RADIUS_FOR_GRADIENT;
double re_forLaplacian     = RADIUS_FOR_LAPLACIAN;
double reMax               = std::max({re_forNumberDensity, re_forGradient, re_forLaplacian});

// physical properties
double fluidDensity = FLUID_DENSITY;

// constant parameters
double n0_forNumberDensity;
double n0_forGradient;
double n0_forLaplacian;
double lambda;

std::vector<Particle> particles;
Eigen::SparseMatrix<double, Eigen::RowMajor> coefficientMatrix;
Eigen::VectorXd sourceTerm;
Eigen::VectorXd pressure;
Bucket bucket;
Domain domain;

int main(int argc, char** argv) {
	startSimulation(simStartTime);

	// omp_set_num_threads(NUMBER_OF_PHYSICAL_CORES);
	readData();
	setParameters();
	bucket.set(reMax, CFL_CONDITION, domain, particles.size());
	writeData();

	while (Time < FINISH_TIME) {
		timestepStartTime = std::clock();

		bucket.storeParticles(particles, domain);
		setNeighbors();
		calGravity();
		calViscosity();
		moveParticle();

		bucket.storeParticles(particles, domain);
		setNeighbors();
		collision();

		bucket.storeParticles(particles, domain);
		setNeighbors();
		calPressure();
		calPressureGradient();
		moveParticleUsingPressureGradient();

		calCourant();

		timestep++;
		Time += DT;
		timestepEndTime = std::clock();
		writeData();
	}

	endSimulation(simStartTime, logFile);

	return 0;
}

void setParameters() {
	// char filename[256];
	// sprintf(filename, "result/result.log");
	// int fileOpenError = fopen_s(&logFile, filename, "w");
	// if (fileOpenError) {
	// 	std::cerr << "cannot write result/result.log" << std::endl;
	// 	std::exit(-1);
	// }

	int iZ_start, iZ_end;
	if (DIM == 2) {
		iZ_start = 0;
		iZ_end   = 1;
	} else {
		iZ_start = -4;
		iZ_end   = 5;
	}

	n0_forNumberDensity = 0.0;
	n0_forGradient      = 0.0;
	n0_forLaplacian     = 0.0;
	lambda              = 0.0;
	for (int iX = -4; iX < 5; iX++) {
		for (int iY = -4; iY < 5; iY++) {
			for (int iZ = iZ_start; iZ < iZ_end; iZ++) {
				if (((iX == 0) && (iY == 0)) && (iZ == 0))
					continue;

				double xj = PARTICLE_DISTANCE * (double) (iX);
				double yj = PARTICLE_DISTANCE * (double) (iY);
				double zj = PARTICLE_DISTANCE * (double) (iZ);
				Eigen::Vector3d rj(xj, yj, zj);
				double dis2 = rj.squaredNorm();
				double dis  = rj.norm();
				n0_forNumberDensity += weight(dis, re_forNumberDensity);
				n0_forGradient += weight(dis, re_forGradient);
				n0_forLaplacian += weight(dis, re_forLaplacian);
				lambda += dis2 * weight(dis, re_forLaplacian);
			}
		}
	}
	lambda = lambda / n0_forLaplacian;
}

void calGravity() {
#pragma omp parallel for
	for (auto& p : particles) {
		if (p.type == ParticleType::Fluid) {
			p.acceleration += G;

		} else {
			p.acceleration.setZero();
		}
	}
}

void calViscosity() {
	double a = (KINEMATIC_VISCOSITY) * (2.0 * DIM) / (n0_forLaplacian * lambda);

#pragma omp parallel for
	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		Eigen::Vector3d viscosityTerm = Eigen::Vector3d::Zero();

		for (auto& neighbor : pi.neighbors) {
			auto& pj = particles[neighbor.id];

			if (neighbor.distance < re_forLaplacian) {
				double w = weight(neighbor.distance, re_forLaplacian);
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
			p.velocity += p.acceleration * DT;
			p.position += p.velocity * DT;

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

			if (neighbor.distance < COLLISION_DISTANCE) {

				double invMi = pi.inverseDensity(fluidDensity);
				double invMj = pj.inverseDensity(fluidDensity);
				double mass  = 1.0 / (invMi + invMj);

				Eigen::Vector3d normal = (pj.position - pi.position).normalized();
				double relVel          = (pj.velocity - pi.velocity).dot(normal);
				double impulse         = 0.0;
				if (relVel < 0.0)
					impulse = -(1 + COEFFICIENT_OF_RESTITUTION) * relVel * mass;
				pi.velocity -= impulse * invMi * normal;
				pj.velocity += impulse * invMj * normal;

				double depth           = COLLISION_DISTANCE - neighbor.distance;
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
	calNumberDensity();
	setBoundaryCondition();
	setSourceTerm();
	setMatrix();
	solveSimultaneousEquations();
	removeNegativePressure();
	setMinimumPressure();
}

void calNumberDensity() {
#pragma omp parallel for
	for (auto& pi : particles) {
		pi.numberDensity = 0.0;

		if (pi.type == ParticleType::Ghost)
			continue;

		for (auto& neighbor : pi.neighbors)
			pi.numberDensity += weight(neighbor.distance, re_forNumberDensity);
	}
}

void setBoundaryCondition() {
	double n0   = n0_forNumberDensity;
	double beta = THRESHOLD_RATIO_OF_NUMBER_DENSITY;

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
	double n0    = n0_forNumberDensity;
	double gamma = RELAXATION_COEFFICIENT_FOR_PRESSURE;

#pragma omp parallel for
	for (auto& pi : particles) {
		pi.sourceTerm = 0.0;
		if (pi.boundaryCondition == FluidState::Inner) {
			pi.sourceTerm = gamma * (1.0 / (DT * DT)) * ((pi.numberDensity - n0) / n0);
		}
	}
}

void setMatrix() {
	std::vector<Eigen::Triplet<double>> triplets;
	auto n0 = n0_forLaplacian;
	auto a  = 2.0 * DIM / (n0 * lambda);
	coefficientMatrix.resize(particles.size(), particles.size());

	for (auto& pi : particles) {
		if (pi.boundaryCondition != FluidState::Inner)
			continue;

		double coefficient_ii = 0.0;
		for (auto& neighbor : pi.neighbors) {
			Particle& pj = particles[neighbor.id];
			if (pj.boundaryCondition == FluidState::Ignored)
				continue;

			if (neighbor.distance < re_forLaplacian) {
				double coefficient_ij = a * weight(neighbor.distance, re_forLaplacian) / fluidDensity;
				triplets.emplace_back(pi.id, pj.id, -1.0 * coefficient_ij);
				coefficient_ii += coefficient_ij;
			}
		}
		coefficient_ii += (COMPRESSIBILITY) / (DT * DT);
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

void setMinimumPressure() {
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

			if (neighbor.distance < re_forGradient) {
				pi.minimumPressure = std::min(pi.minimumPressure, pj.pressure);
				pj.minimumPressure = std::min(pj.minimumPressure, pi.pressure);
			}
		}
	}
}

void calPressureGradient() {
	double a = DIM / n0_forGradient;

#pragma omp parallel for
	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		Eigen::Vector3d grad = Eigen::Vector3d::Zero();
		for (auto& neighbor : pi.neighbors) {
			Particle& pj = particles[neighbor.id];
			if (pj.type == ParticleType::Ghost || pj.type == ParticleType::DummyWall)
				continue;

			if (neighbor.distance < re_forGradient) {
				double w = weight(neighbor.distance, re_forGradient);
				// double dist2 = pow(neighbor.distance, 2);
				double dist2 = (pj.position - pi.position).squaredNorm();
				double pij   = (pj.pressure - pi.minimumPressure) / dist2;
				grad += (pj.position - pi.position) * pij * w;
			}
		}
		grad *= a;
		pi.acceleration -= grad * pi.inverseDensity(fluidDensity);
	}
}

void moveParticleUsingPressureGradient() {
#pragma omp parallel for
	for (auto&& p : particles) {
		if (p.type == ParticleType::Fluid) {
			p.velocity += p.acceleration * DT;
			p.position += p.acceleration * DT * DT;
		}

		p.acceleration.setZero();
	}
}

void calCourant() {
	courant = 0.0;

	for (auto& pi : particles) {
		if (pi.type != ParticleType::Fluid)
			continue;

		double iCourant = (pi.velocity.norm() * DT) / PARTICLE_DISTANCE;
		courant         = std::max(courant, iCourant);
	}

	if (courant > CFL_CONDITION) {
		std::cerr << "ERROR: Courant number is larger than CFL condition. Courant = " << courant << std::endl;
	}
}

void readData() {
	std::stringstream ss;
	std::ifstream ifs;

	ss.str("./input/input.prof");
	ifs.open(ss.str());
	if (ifs.fail()) {
		std::cerr << "cannot read " << ss.str() << std::endl;
		std::exit(-1);
	}

	int particleSize;
	ifs >> Time;
	initialTime = Time;
	ifs >> particleSize;
	rep(i, 0, particleSize) {
		int type_int;
		ParticleType type;
		Eigen::Vector3d pos, vel;

		ifs >> type_int;
		ifs >> pos.x() >> pos.y() >> pos.z();
		ifs >> vel.x() >> vel.y() >> vel.z();

		type = static_cast<ParticleType>(type_int);
		if (type != ParticleType::Ghost) {
			particles.emplace_back(particles.size(), type, pos, vel);
		}
	}
	ifs.close();
	ifs.clear();

	ss.str("./input/input.domain");
	ifs.open(ss.str());
	if (ifs.fail()) {
		std::cerr << "cannot read " << ss.str() << std::endl;
		std::exit(-1);
	}

	std::string dumstr;
	ifs >> dumstr >> domain.xMin;
	ifs >> dumstr >> domain.xMax;
	ifs >> dumstr >> domain.yMin;
	ifs >> dumstr >> domain.yMax;
	ifs >> dumstr >> domain.zMin;
	ifs >> dumstr >> domain.zMax;
	domain.xLength = domain.xMax - domain.xMin;
	domain.yLength = domain.yMax - domain.yMin;
	domain.zLength = domain.zMax - domain.zMin;
	ifs.close();
	ifs.clear();
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
	second = ((FINISH_TIME - Time) / Time) * ave * timestep;
	calSecondMinuteHour(second, minute, hour);
	if (timestep == 0)
		sprintf(remain, "remain=-h --m --s");
	else
		sprintf(remain, "remain=%dh %02dm %02ds", hour, minute, second);

	double last = (double) (timestepEndTime - timestepStartTime) / CLOCKS_PER_SEC;

	// terminal output
	printf("%d: dt=%.gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
	       "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
	       timestep, DT, Time, FINISH_TIME, elapsed, remain, ave, last, fileNumber, courant);

	// log output
	// fprintf(logFile,
	//         "%d: dt=%gs   t=%.3lfs   fin=%.1lfs   %s   %s   ave=%.3lfs/step   "
	//         "last=%.3lfs/step   out=%dfiles   Courant=%.2lf\n",
	//         timestep, DT, Time, FINISH_TIME, elapsed, remain, ave, last, fileNumber, courant);

	// error output
	fprintf(stderr, "%4d: t=%.3lfs\n", timestep, Time);

	if (Time - initialTime >= OUTPUT_PERIOD * double(fileNumber)) {
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

void setNeighbors() {
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
						if (j != pi.id && dist < reMax) {
							pi.neighbors.emplace_back(j, dist);
						}

						j = bucket.next[j];
					}
				}
			}
		}
	}
}

double weight(double dis, double re) {
	if (dis >= re) {
		return 0.0;

	} else {
		return (re / dis) - 1.0;
	}
}
