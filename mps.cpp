#include "eigen/Eigen/Dense"
#include "eigen/Eigen/Sparse"
#include <iostream>
#include <string>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <queue>
#include <set>
#include <sstream>
// for 2D
constexpr int DIM = 2;
constexpr double PARTICLE_DISTANCE = 0.025;
constexpr double DT = 0.001;
constexpr int OUTPUT_INTERVAL = 20;
const Eigen::Vector3d G(0.0, -9.8, 0.0);

// for 3D
/*
#define DIM                  3
#define PARTICLE_DISTANCE    0.075
#define DT                   0.003
#define OUTPUT_INTERVAL      2
const Eigen::Vector3d G(0.0, 0.0, -9.8);
*/

// computational condition
constexpr int ARRAY_SIZE = 5000;
constexpr double FINISH_TIME = 2.0;

// effective radius
constexpr double RADIUS_FOR_NUMBER_DENSITY = 2.1 * PARTICLE_DISTANCE;
constexpr double RADIUS_FOR_GRADIENT = 2.1 * PARTICLE_DISTANCE;
constexpr double RADIUS_FOR_LAPLACIAN = 3.1 * PARTICLE_DISTANCE;

// physical properties
constexpr double KINEMATIC_VISCOSITY = 1.0e-6;
constexpr double FLUID_DENSITY = 1000.0;

// free surface detection
constexpr double THRESHOLD_RATIO_OF_NUMBER_DENSITY = 0.97;
constexpr int GHOST_OR_DUMMY = -1;
constexpr int SURFACE = 1;
constexpr int INNER_PARTICLE = 0;

// boundary condition
constexpr int DIRICHLET_BOUNDARY_IS_NOT_CONNECTED = 0;
constexpr int DIRICHLET_BOUNDARY_IS_CONNECTED = 1;
constexpr int DIRICHLET_BOUNDARY_IS_CHECKED = 2;

// particle collision
constexpr double COLLISION_DISTANCE = 0.5 * PARTICLE_DISTANCE;
constexpr double COEFFICIENT_OF_RESTITUTION = 0.2;

// params for pressure Poisson equation
constexpr double COMPRESSIBILITY = 0.45E-9;
constexpr double RELAXATION_COEFFICIENT_FOR_PRESSURE = 0.2;

// for computation
constexpr double EPS = (0.01 * PARTICLE_DISTANCE);

enum class ParticleType {
	Ghost,
	Fluid,
	Wall,
	DummyWall,
};

enum class FluidState {
	Ignored,        // Ghost or dummy
	FreeSurface,    // free surface particle
	SubFreeSurface, // inner particle near free surface
	Inner,          // inner fluid particle
	Splash          // splash fluid particle
};

class Particle {
private:
public:
	ParticleType particleType = ParticleType::Ghost;
	Eigen::Vector3d position;
	Eigen::Vector3d velocity;
	Eigen::Vector3d acceleration;
	double pressure = 0;
	double numberDensity = 0;
	FluidState boundaryCondition = FluidState::Ignored;
	double sourceTerm = 0;
	double minimumPressure = 0;

	Particle(double x, double y, double z, ParticleType type) {
		position << x, y, z;
		velocity.setZero();
		acceleration.setZero();
		particleType = type;
	}

	double inverseDensity() const {
		switch (particleType) {
		case ParticleType::Ghost:
			return 1.0 / 0;
		case ParticleType::Fluid:
			return 1 / FLUID_DENSITY;
		case ParticleType::Wall:
		case ParticleType::DummyWall:
			return 0;
		}
	}
};

std::vector<Particle> particles;
Eigen::SparseMatrix<double, Eigen::RowMajor> coefficientMatrix;
Eigen::VectorXd sourceTerm;
Eigen::VectorXd pressure;

void initializeParticlePositionAndVelocity_for2dim();
void initializeParticlePositionAndVelocity_for3dim();
void calConstantParameter();
void calNZeroAndLambda();
double weight(double distance, double re);
void mainLoopOfSimulation();
void calGravity();
void calViscosity();
void moveParticle();
void collision();
void calPressure();
void calNumberDensity();
void setBoundaryCondition();
void setSourceTerm();
void setMatrix();
void exceptionalProcessingForBoundaryCondition();
void solveSimultaneousEquations();
void removeNegativePressure();
void setMinimumPressure();
void calPressureGradient();
void moveParticleUsingPressureGradient();
void writeData_inProfFormat();
void writeData_inVtuFormat();

int fileNumber;
double Time;
// int numberOfParticles; // use particles.size()
double re_forNumberDensity, re2_forNumberDensity;
double re_forGradient, re2_forGradient;
double re_forLaplacian, re2_forLaplacian;
double n0_forNumberDensity;
double n0_forGradient;
double n0_forLaplacian;
double lambda;
double collisionDistance, collisionDistance2;
double fluidDensity = FLUID_DENSITY;

int main(int argc, char** argv) {
	printf("\n*** START PARTICLE-SIMULATION ***\n");

	if (DIM == 2) {
		initializeParticlePositionAndVelocity_for2dim();

	} else {
		initializeParticlePositionAndVelocity_for3dim();
	}

	calConstantParameter();

	mainLoopOfSimulation();

	printf("*** END ***\n\n");

	return 0;
}

void initializeParticlePositionAndVelocity_for2dim() {

	int nX = (int)(1.0 / PARTICLE_DISTANCE) + 5;
	int nY = (int)(0.6 / PARTICLE_DISTANCE) + 5;
	for (int iX = -4; iX < nX; iX++) {
		for (int iY = -4; iY < nY; iY++) {
			double x = PARTICLE_DISTANCE * (double)(iX);
			double y = PARTICLE_DISTANCE * (double)(iY);
			ParticleType type = ParticleType::Ghost;

			/* dummy wall region */
			if (((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) {
				type = ParticleType::DummyWall;
			}

			/* wall region */
			if (((x > -2.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 2.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) {
				type = ParticleType::Wall;
			}

			/* wall region */
			if (((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.6 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) {
				type = ParticleType::Wall;
			}

			/* empty region */
			if (((x > 0.0 + EPS) && (x <= 1.00 + EPS)) && (y > 0.0 + EPS)) {
				type = ParticleType::Ghost;
			}

			/* fluid region */
			if (((x > 0.0 + EPS) && (x <= 0.25 + EPS)) && ((y > 0.0 + EPS) && (y <= 0.50 + EPS))) {
				type = ParticleType::Fluid;
			}
			if (type != ParticleType::Ghost) {
				particles.emplace_back(x, y, 0.0, type);
			}
		}
	}
}

void initializeParticlePositionAndVelocity_for3dim() {
	int nX = (int)(1.0 / PARTICLE_DISTANCE) + 5;
	int nY = (int)(0.6 / PARTICLE_DISTANCE) + 5;
	int nZ = (int)(0.3 / PARTICLE_DISTANCE) + 5;
	for (int iX = -4; iX < nX; iX++) {
		for (int iY = -4; iY < nY; iY++) {
			for (int iZ = -4; iZ < nZ; iZ++) {
				double x = PARTICLE_DISTANCE * iX;
				double y = PARTICLE_DISTANCE * iY;
				double z = PARTICLE_DISTANCE * iZ;
				ParticleType type = ParticleType::Ghost;

				/* dummy wall region */
				if ((((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (z <= 0.3 + 4.0 * PARTICLE_DISTANCE + EPS))) {
					type = ParticleType::DummyWall;
				}

				/* wall region */
				if ((((x > -2.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 2.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) && (z <= 0.3 + 2.0 * PARTICLE_DISTANCE + EPS))) {
					type = ParticleType::Wall;
				}

				/* wall region */
				if ((((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.6 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (z <= 0.3 + 4.0 * PARTICLE_DISTANCE + EPS))) {
					type = ParticleType::Wall;
				}

				/* fluid region */
				if ((((x > 0.0 + EPS) && (x <= 0.25 + EPS)) && ((y > 0.0 + EPS) && (y < 0.5 + EPS))) &&
				    ((z > 0.0 + EPS) && (z <= 0.3 + EPS))) {
					type = ParticleType::Fluid;
				}

				if (type != ParticleType::Ghost) {
					particles.emplace_back(x, y, z, type);
				}
			}
		}
	}
}

void calConstantParameter() {
	re_forNumberDensity = RADIUS_FOR_NUMBER_DENSITY;
	re_forGradient = RADIUS_FOR_GRADIENT;
	re_forLaplacian = RADIUS_FOR_LAPLACIAN;
	re2_forNumberDensity = re_forNumberDensity * re_forNumberDensity;
	re2_forGradient = re_forGradient * re_forGradient;
	re2_forLaplacian = re_forLaplacian * re_forLaplacian;

	calNZeroAndLambda();

	fluidDensity = FLUID_DENSITY;

	collisionDistance = COLLISION_DISTANCE;
	collisionDistance2 = collisionDistance * collisionDistance;

	fileNumber = 0;
	Time = 0.0;
}

void calNZeroAndLambda() {
	int iZ_start, iZ_end;
	if (DIM == 2) {
		iZ_start = 0;
		iZ_end = 1;
	} else {
		iZ_start = -4;
		iZ_end = 5;
	}

	n0_forNumberDensity = 0.0;
	n0_forGradient = 0.0;
	n0_forLaplacian = 0.0;
	lambda = 0.0;
	for (int iX = -4; iX < 5; iX++) {
		for (int iY = -4; iY < 5; iY++) {
			for (int iZ = iZ_start; iZ < iZ_end; iZ++) {
				if (((iX == 0) && (iY == 0)) && (iZ == 0))
					continue;

				double xj = PARTICLE_DISTANCE * (double)(iX);
				double yj = PARTICLE_DISTANCE * (double)(iY);
				double zj = PARTICLE_DISTANCE * (double)(iZ);
				Eigen::Vector3d rj(xj, yj, zj);
				double dis2 = rj.squaredNorm();
				double dis = rj.norm();
				n0_forNumberDensity += weight(dis, re_forNumberDensity);
				n0_forGradient += weight(dis, re_forGradient);
				n0_forLaplacian += weight(dis, re_forLaplacian);
				lambda += dis2 * weight(dis, re_forLaplacian);
			}
		}
	}
	lambda = lambda / n0_forLaplacian;

	std::cout << "n0=" << n0_forNumberDensity << std::endl;
}

double weight(double dis, double re) {
	if (dis >= re) {
		return 0.0;

	} else {
		return (re / dis) - 1.0;
	}
}

void mainLoopOfSimulation() {
	int iTimeStep = 0;

	writeData_inVtuFormat();
	writeData_inProfFormat();

	while (1) {
		calGravity();
		calViscosity();
		moveParticle();

		collision();

		calPressure();
		calPressureGradient();
		moveParticleUsingPressureGradient();

		iTimeStep++;
		Time += DT;
		if ((iTimeStep % OUTPUT_INTERVAL) == 0) {
			printf("TimeStepNumber: %4d   time: %lf(s)   numberOfParticles: %d\n", iTimeStep, Time, particles.size());
			writeData_inVtuFormat();
			writeData_inProfFormat();
		}

		if (Time >= FINISH_TIME) {
			break;
		}
	}
}

void calGravity() {
	for (int i = 0; i < particles.size(); i++) {
		if (particles[i].particleType == ParticleType::Fluid) {
			particles[i].acceleration += G;

		} else {
			particles[i].acceleration.setZero();
		}
	}
}

void calViscosity() {
	double a = (KINEMATIC_VISCOSITY) * (2.0 * DIM) / (n0_forLaplacian * lambda);
	size_t n = particles.size();
	for (int i = 0; i < n; i++) {
		auto& pi = particles[i];
		if (pi.particleType == ParticleType::Ghost)
			continue;
		Eigen::Vector3d viscosityTerm = Eigen::Vector3d::Zero();

		for (int j = 0; i < n; i++) {
			auto& pj = particles[j];
			if (i == j || pj.particleType == ParticleType::Ghost)
				continue;
			auto diff = pj.position - pi.position;
			auto distance2 = diff.squaredNorm();
			double distance = sqrt(distance2);
			if (distance < re_forLaplacian) {
				double w = weight(distance, re_forLaplacian);
				viscosityTerm += (pj.velocity - pi.velocity) * w;
			}
		}

		viscosityTerm *= a;
		pi.acceleration += viscosityTerm;
	}
}

void moveParticle() {
	for (size_t i = 0; i < particles.size(); i++) {
		if (particles[i].particleType == ParticleType::Fluid) {
			particles[i].velocity += particles[i].acceleration * DT;
			particles[i].position += particles[i].velocity * DT;
		} else {
			particles[i].acceleration.setZero();
		}
	}
}

void collision() {
	double e = COEFFICIENT_OF_RESTITUTION;

	size_t n = particles.size();
	for (int i = 1; i < n; i++) {
		for (int j = 0; j < i; j++) {
			auto& p1 = particles[i];
			auto& p2 = particles[j];
			auto t1 = p1.particleType;
			auto t2 = p2.particleType;
			// do not collide with ghost particles
			if (t1 == ParticleType::Ghost || t2 == ParticleType::Ghost)
				continue;
			// at least one particle must be a fluid particle
			if (t1 != ParticleType::Fluid && t2 != ParticleType::Fluid)
				continue;
			auto diff = p1.position - p2.position;
			double dist2 = diff.squaredNorm();
			if (dist2 < collisionDistance2) {
				double dist = sqrt(dist2);
				auto normal = diff.normalized();
				double depth = collisionDistance - dist;
				double invM1 = p1.inverseDensity();
				double invM2 = p2.inverseDensity();
				double mass = 1.0 / (invM1 + invM2);
				double rvn = (p1.velocity - p2.velocity).dot(normal);
				double impulse = rvn > 0 ? 0 : -(1 + e) * rvn * mass;
				double positionImpulse = depth * mass;
				p1.velocity += impulse * invM1 * normal;
				p2.velocity -= impulse * invM2 * normal;
				p1.position += positionImpulse * invM1 * normal;
				p2.position -= positionImpulse * invM2 * normal;
				std::cerr << "WARNING: Collision between particles " << i << " and " << j << " occurred." << std::endl;
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
	for (size_t i = 0; i < particles.size(); i++) {
		particles[i].numberDensity = 0.0;
		if (particles[i].particleType == ParticleType::Ghost) {
			continue;
		}
		for (size_t j = 0; j < particles.size(); j++) {
			if ((j == i) || (particles[j].particleType == ParticleType::Ghost)) {
				continue;
			}
			auto r_ij = particles[j].position - particles[i].position;
			auto norm_r_ij = r_ij.norm();
			particles[i].numberDensity += weight(norm_r_ij, re_forNumberDensity);
		}
	}
}

void setBoundaryCondition() {
	double n0 = n0_forNumberDensity;
	double beta = THRESHOLD_RATIO_OF_NUMBER_DENSITY;

	for (size_t i = 0; i < particles.size(); i++) {
		if (particles[i].particleType == ParticleType::Ghost || particles[i].particleType == ParticleType::DummyWall) {
			particles[i].boundaryCondition = FluidState::Ignored;
		} else if (particles[i].numberDensity < beta * n0) {
			particles[i].boundaryCondition = FluidState::FreeSurface;
		} else {
			particles[i].boundaryCondition = FluidState::Inner;
		}
	}
}

void setSourceTerm() {
	int i;
	double n0 = n0_forNumberDensity;
	double gamma = RELAXATION_COEFFICIENT_FOR_PRESSURE;

	for (size_t i = 0; i < particles.size(); i++) {
		particles[i].sourceTerm = 0.0;
		if (particles[i].boundaryCondition == FluidState::Inner) {
			particles[i].sourceTerm = gamma * (1.0 / (DT * DT)) * ((particles[i].numberDensity - n0) / n0);
		}
	}
}

void setMatrix() {
	std::vector<Eigen::Triplet<double>> triplets;
	auto n0 = n0_forLaplacian;

	auto a = 2.0 * DIM / (n0 * lambda);
	for (size_t i = 0; i < particles.size(); i++) {
		double coefficient_ii = 0;
		if (particles[i].boundaryCondition != FluidState::Inner) {
			continue;
		}
		for (size_t j = 0; j < particles.size(); j++) {
			if ((j == i) || (particles[j].boundaryCondition == FluidState::Ignored)) {
				continue;
			}
			auto r_ij = particles[j].position - particles[i].position;
			auto square_r_ij = r_ij.squaredNorm();
			if (square_r_ij > re_forLaplacian * re_forLaplacian) {
				continue;
			}
			auto norm_r_ij = r_ij.norm();
			double coefficient_ij = a * weight(norm_r_ij, re_forLaplacian) / fluidDensity;
			triplets.emplace_back(i, j, -1.0 * coefficient_ij);
			coefficient_ii += coefficient_ij;
		}
		coefficient_ii += (COMPRESSIBILITY) / (DT * DT);
		triplets.emplace_back(i, i, coefficient_ii);
	}
	coefficientMatrix.resize(particles.size(), particles.size());
	coefficientMatrix.setFromTriplets(triplets.begin(), triplets.end());
	// exceptionalProcessingForBoundaryCondition();
}

void exceptionalProcessingForBoundaryCondition() {
	/* If there is no Dirichlet boundary condition on the fluid,
	   increase the diagonal terms of the matrix for an exception. This allows
	   us to solve the matrix without Dirichlet boundary conditions. */
	std::vector<bool> checked(particles.size(), false);
	std::vector<bool> connected(particles.size(), false);

	for (size_t i = 0; i < particles.size(); i++) {
		if (particles[i].boundaryCondition == FluidState::FreeSurface) {
			connected[i] = true;
		}
		if (connected[i]) {
			continue;
		}
		// BFS for connected components
		std::queue<size_t> queue;
		queue.push(i);
		checked[i] = true;
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
						checked[v] = true;
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
	for (size_t i = 0; i < particles.size(); i++) {
		sourceTerm[i] = particles[i].sourceTerm;
	}

	Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> solver;
	solver.compute(coefficientMatrix);
	pressure = solver.solve(sourceTerm);
	if (solver.info() != Eigen::Success) {
		std::cerr << "Pressure calculation failed." << std::endl;
	}

	for (size_t i = 0; i < particles.size(); i++) {
		particles[i].pressure = pressure[i];
	}
}

void removeNegativePressure() {
	for (auto& p : particles) {
		if (p.pressure < 0) {
			p.pressure = 0;
		}
	}
}

void setMinimumPressure() {
	for (auto& p : particles) {
		p.minimumPressure = p.pressure;
	}
	size_t n = particles.size();
	for (int i = 1; i < n; i++) {
		auto& p1 = particles[i];
		auto t1 = p1.particleType;
		if (t1 == ParticleType::Ghost || t1 == ParticleType::DummyWall)
			continue;
		for (int j = 0; j < i; j++) {
			auto& p2 = particles[j];
			auto t2 = p2.particleType;
			if (t2 == ParticleType::Ghost || t2 == ParticleType::DummyWall)
				continue;
			auto diff = p1.position - p2.position;
			if (diff.squaredNorm() > re2_forGradient)
				continue;
			p1.minimumPressure = std::min(p1.minimumPressure, p2.pressure);
			p2.minimumPressure = std::min(p2.minimumPressure, p1.pressure);
		}
	}
}

void calPressureGradient() {
	double a = DIM / n0_forGradient;
	size_t n = particles.size();

	for (int i = 0; i < n; i++) {
		auto& p1 = particles[i];
		if (p1.particleType != ParticleType::Fluid)
			continue;
		Eigen::Vector3d grad = Eigen::Vector3d::Zero();
		for (int j = 0; j < n; j++) {
			if (i == j)
				continue;
			auto& p2 = particles[j];
			if (p2.particleType == ParticleType::Ghost || p2.particleType == ParticleType::DummyWall)
				continue;
			auto diff = p2.position - p1.position;
			double dist2 = diff.squaredNorm();
			if (dist2 < re2_forGradient) {
				double dist = sqrt(dist2);
				double w = weight(dist, re_forGradient);
				double pij = (p2.pressure - p1.minimumPressure) / dist2;
				grad += diff * pij * w;
			}
		}
		grad *= a;
		p1.acceleration -= grad * p1.inverseDensity();
	}
}

void moveParticleUsingPressureGradient() {
	for (auto&& p : particles) {
		if (p.particleType == ParticleType::Fluid) {
			p.velocity += p.acceleration * DT;
			p.position += p.acceleration * DT * DT;
		}
		p.acceleration.setZero();
	}
}

void writeData_inProfFormat() {
	std::stringstream ss;
	ss << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".prof";

	std::ofstream ofs(ss.str());
	if (ofs.fail()) {
		std::cerr << "cannot write " << ss.str() << std::endl;
	}
	ofs << Time << std::endl;
	ofs << particles.size() << std::endl;
	for (size_t i = 0; i < particles.size(); i++) {
		for (size_t j = 0; j < 3; j++) {
			ofs << particles[i].position[j] << " ";
		}
		for (size_t j = 0; j < 3; j++) {
			ofs << particles[i].velocity[j] << " ";
		}
		ofs << particles[i].pressure << " " << particles[i].numberDensity << std::endl;
	}
	fileNumber++;
}

void writeData_inVtuFormat() {
	std::stringstream ss;
	ss << "output_" << std::setfill('0') << std::setw(4) << fileNumber << ".vtu";

	std::ofstream ofs(ss.str());
	if (ofs.fail()) {
		std::cerr << "cannot write " << ss.str() << std::endl;
	}
	ofs << "<?xml version='1.0' encoding='UTF-8'?>" << std::endl;
	ofs << "<VTKFile xmlns='VTK' byte_order='LittleEndian' version='0.1' "
	       "type='UnstructuredGrid'>"
	    << std::endl;
	ofs << "<UnstructuredGrid>" << std::endl;
	ofs << "<Piece NumberOfCells='" << particles.size() << "' NumberOfPoints='" << particles.size() << "'>\n";
	ofs << "<Points>" << std::endl;
	ofs << "<DataArray NumberOfComponents='3' type='Float64' "
	       "Name='position' format='ascii'>"
	    << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].position[0] << " " << particles[i].position[1] << " " << particles[i].position[2]
		    << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "</Points>" << std::endl;
	ofs << "<PointData>" << std::endl;
	ofs << "<DataArray NumberOfComponents='1' type='Int32' "
	       "Name='particleType' format='ascii'>"
	    << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << static_cast<int>(particles[i].particleType) << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "<DataArray NumberOfComponents='3' type='Float32' "
	       "Name='velocity' format='ascii'>"
	    << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].velocity[0] << " " << particles[i].velocity[1] << " " << particles[i].velocity[2]
		    << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "<DataArray NumberOfComponents='1' type='Float64' "
	       "Name='pressure' format='ascii'>"
	    << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << particles[i].pressure << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "</PointData>" << std::endl;
	ofs << "<Cells>" << std::endl;
	ofs << "<DataArray type='Int32' Name='connectivity' format='ascii'>" << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << i << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "<DataArray type='Int32' Name='offsets' format='ascii'>" << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << i + 1 << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "<DataArray type='UInt8' Name='types' format='ascii'>" << std::endl;
	for (int i = 0; i < particles.size(); i++) {
		ofs << "1" << std::endl;
	}
	ofs << "</DataArray>" << std::endl;
	ofs << "</Cells>" << std::endl;
	ofs << "</Piece>" << std::endl;
	ofs << "</UnstructuredGrid>" << std::endl;
	ofs << "</VTKFile>" << std::endl;
}
