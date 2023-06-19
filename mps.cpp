#include "eigen/Eigen/Dense"
#include <iostream>
#include <string.h>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>

constexpr int DIM = 2;
constexpr double PARTICLE_DISTANCE = 0.025;
constexpr double DT = 0.001;
constexpr int OUTPUT_INTERVAL = 20;

/* for three-dimensional simulation */
/*
#define DIM                  3
#define PARTICLE_DISTANCE    0.075
#define DT                   0.003
#define OUTPUT_INTERVAL      2
*/

// computational condition
constexpr int ARRAY_SIZE = 5000;
constexpr double FINISH_TIME = 2.0;
constexpr double G_X = 0.0;
constexpr double G_Y = -9.8;
constexpr double G_Z = 0.0;

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
constexpr bool ON = true;
constexpr bool OFF = false;

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
	Eigen::Vector3d acceleration;
	Eigen::Vector3d velocity;
	Eigen::Vector3d position;
	double pressure = 0;
	double numberDensity = 0;
	FluidState boundaryCondition = FluidState::Ignored;
	double sourceTerm = 0;
	double minimumPressure = 0;

	Particle() {
		//
	}

	void init(double x, double y, double z, ParticleType type) {
		position << x, y, z;
		particleType = type;
	}
};

std::vector<Particle> particles;

void initializeParticlePositionAndVelocity_for2dim(void);
void initializeParticlePositionAndVelocity_for3dim(void);
void calConstantParameter(void);
void calNZeroAndLambda(void);
double weight(double distance, double re);
void mainLoopOfSimulation(void);
void calGravity(void);
void calViscosity(void);
void moveParticle(void);
void collision(void);
void calPressure(void);
void calNumberDensity(void);
void setBoundaryCondition(void);
void setSourceTerm(void);
void setMatrix(void);
void exceptionalProcessingForBoundaryCondition(void);
void checkBoundaryCondition(void);
void increaseDiagonalTerm(void);
void solveSimultaneousEquationsByGaussEliminationMethod(void);
void removeNegativePressure(void);
void setMinimumPressure(void);
void calPressureGradient(void);
void moveParticleUsingPressureGradient(void);
void writeData_inProfFormat(void);
void writeData_inVtuFormat(void);

double acceleration[3 * ARRAY_SIZE];
ParticleType particleType[ARRAY_SIZE];
double position[3 * ARRAY_SIZE];
double velocity[3 * ARRAY_SIZE];
double pressure[ARRAY_SIZE];
double numberDensity[ARRAY_SIZE];
FluidState boundaryCondition[ARRAY_SIZE];
double sourceTerm[ARRAY_SIZE];
int flagForCheckingBoundaryCondition[ARRAY_SIZE];
double coefficientMatrix[ARRAY_SIZE * ARRAY_SIZE];
double minimumPressure[ARRAY_SIZE];
int fileNumber;
double Time;
int numberOfParticles;
double re_forNumberDensity, re2_forNumberDensity;
double re_forGradient, re2_forGradient;
double re_forLaplacian, re2_forLaplacian;
double n0_forNumberDensity;
double n0_forGradient;
double n0_forLaplacian;
double lambda;
double collisionDistance, collisionDistance2;
double fluidDensity;

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

void initializeParticlePositionAndVelocity_for2dim(void) {
	int iX, iY;
	int nX, nY;
	double x, y, z;
	int i = 0;
	int flagOfParticleGeneration;

	nX = (int)(1.0 / PARTICLE_DISTANCE) + 5;
	nY = (int)(0.6 / PARTICLE_DISTANCE) + 5;
	for (iX = -4; iX < nX; iX++) {
		for (iY = -4; iY < nY; iY++) {
			x = PARTICLE_DISTANCE * (double)(iX);
			y = PARTICLE_DISTANCE * (double)(iY);
			z = 0.0;
			flagOfParticleGeneration = OFF;

			/* dummy wall region */
			if (((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) {
				particleType[i] = ParticleType::DummyWall;
				flagOfParticleGeneration = ON;
			}

			/* wall region */
			if (((x > -2.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 2.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) {
				particleType[i] = ParticleType::Wall;
				flagOfParticleGeneration = ON;
			}

			/* wall region */
			if (((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.6 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) {
				particleType[i] = ParticleType::Wall;
				flagOfParticleGeneration = ON;
			}

			/* empty region */
			if (((x > 0.0 + EPS) && (x <= 1.00 + EPS)) && (y > 0.0 + EPS)) {
				flagOfParticleGeneration = OFF;
			}

			/* fluid region */
			if (((x > 0.0 + EPS) && (x <= 0.25 + EPS)) && ((y > 0.0 + EPS) && (y <= 0.50 + EPS))) {
				particleType[i] = ParticleType::Fluid;
				flagOfParticleGeneration = ON;
			}

			if (flagOfParticleGeneration == ON) {
				position[i * 3] = x;
				position[i * 3 + 1] = y;
				position[i * 3 + 2] = z;
				i++;
			}
		}
	}
	numberOfParticles = i;
	for (i = 0; i < numberOfParticles * 3; i++) {
		velocity[i] = 0.0;
	}
}

void initializeParticlePositionAndVelocity_for3dim(void) {
	int iX, iY, iZ;
	int nX, nY, nZ;
	double x, y, z;
	int i = 0;
	int flagOfParticleGeneration;

	nX = (int)(1.0 / PARTICLE_DISTANCE) + 5;
	nY = (int)(0.6 / PARTICLE_DISTANCE) + 5;
	nZ = (int)(0.3 / PARTICLE_DISTANCE) + 5;
	for (iX = -4; iX < nX; iX++) {
		for (iY = -4; iY < nY; iY++) {
			for (iZ = -4; iZ < nZ; iZ++) {
				x = PARTICLE_DISTANCE * iX;
				y = PARTICLE_DISTANCE * iY;
				z = PARTICLE_DISTANCE * iZ;
				flagOfParticleGeneration = OFF;

				/* dummy wall region */
				if ((((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (z <= 0.3 + 4.0 * PARTICLE_DISTANCE + EPS))) {
					particleType[i] = ParticleType::DummyWall;
					flagOfParticleGeneration = ON;
				}

				/* wall region */
				if ((((x > -2.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 2.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) && (z <= 0.3 + 2.0 * PARTICLE_DISTANCE + EPS))) {
					particleType[i] = ParticleType::Wall;
					flagOfParticleGeneration = ON;
				}

				/* wall region */
				if ((((x > -4.0 * PARTICLE_DISTANCE + EPS) && (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.6 - 2.0 * PARTICLE_DISTANCE + EPS) && (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) && (z <= 0.3 + 4.0 * PARTICLE_DISTANCE + EPS))) {
					particleType[i] = ParticleType::Wall;
					flagOfParticleGeneration = ON;
				}

				/* empty region */
				if ((((x > 0.0 + EPS) && (x <= 1.00 + EPS)) && (y > 0.0 + EPS)) &&
				    ((z > 0.0 + EPS) && (z <= 0.3 + EPS))) {
					flagOfParticleGeneration = OFF;
				}

				/* fluid region */
				if ((((x > 0.0 + EPS) && (x <= 0.25 + EPS)) && ((y > 0.0 + EPS) && (y < 0.5 + EPS))) &&
				    ((z > 0.0 + EPS) && (z <= 0.3 + EPS))) {
					particleType[i] = ParticleType::Fluid;
					flagOfParticleGeneration = ON;
				}

				if (flagOfParticleGeneration == ON) {
					position[i * 3] = x;
					position[i * 3 + 1] = y;
					position[i * 3 + 2] = z;
					i++;
				}
			}
		}
	}
	numberOfParticles = i;
	for (i = 0; i < numberOfParticles * 3; i++) {
		velocity[i] = 0.0;
	}
}

void calConstantParameter(void) {
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

void calNZeroAndLambda(void) {
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
				lambda += dis2* weight(dis, re_forLaplacian);
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

void mainLoopOfSimulation(void) {
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
			printf("TimeStepNumber: %4d   time: %lf(s)   numberOfParticles: %d\n", iTimeStep, Time, numberOfParticles);
			writeData_inVtuFormat();
			writeData_inProfFormat();
		}

		if (Time >= FINISH_TIME) {
			break;
		}
	}
}

void calGravity(void) {
	for (int i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Fluid) {
			acceleration[i * 3] = G_X;
			acceleration[i * 3 + 1] = G_Y;
			acceleration[i * 3 + 2] = G_Z;

		} else {
			acceleration[i * 3] = 0.0;
			acceleration[i * 3 + 1] = 0.0;
			acceleration[i * 3 + 2] = 0.0;
		}
	}
}

void calViscosity(void) {
	double distance, distance2;
	double w;
	double xij, yij, zij;

	double a = (KINEMATIC_VISCOSITY) * (2.0 * DIM) / (n0_forLaplacian * lambda);
	for (int i = 0; i < numberOfParticles; i++) {
		if (particleType[i] != ParticleType::Fluid) continue;
		double viscosityTerm_x = 0.0;
		double viscosityTerm_y = 0.0;
		double viscosityTerm_z = 0.0;

		for (int j = 0; j < numberOfParticles; j++) {
			if ((j == i) || (particleType[j] == ParticleType::Ghost))
				continue;
			xij = position[j * 3] - position[i * 3];
			yij = position[j * 3 + 1] - position[i * 3 + 1];
			zij = position[j * 3 + 2] - position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			if (distance < re_forLaplacian) {
				w = weight(distance, re_forLaplacian);
				viscosityTerm_x += (velocity[j * 3] - velocity[i * 3]) * w;
				viscosityTerm_y += (velocity[j * 3 + 1] - velocity[i * 3 + 1]) * w;
				viscosityTerm_z += (velocity[j * 3 + 2] - velocity[i * 3 + 2]) * w;
			}
		}
		viscosityTerm_x = viscosityTerm_x * a;
		viscosityTerm_y = viscosityTerm_y * a;
		viscosityTerm_z = viscosityTerm_z * a;
		acceleration[i * 3] += viscosityTerm_x;
		acceleration[i * 3 + 1] += viscosityTerm_y;
		acceleration[i * 3 + 2] += viscosityTerm_z;
	}
}

void moveParticle(void) {
	int i;

	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Fluid) {
			velocity[i * 3] += acceleration[i * 3] * DT;
			velocity[i * 3 + 1] += acceleration[i * 3 + 1] * DT;
			velocity[i * 3 + 2] += acceleration[i * 3 + 2] * DT;

			position[i * 3] += velocity[i * 3] * DT;
			position[i * 3 + 1] += velocity[i * 3 + 1] * DT;
			position[i * 3 + 2] += velocity[i * 3 + 2] * DT;
		}
		acceleration[i * 3] = 0.0;
		acceleration[i * 3 + 1] = 0.0;
		acceleration[i * 3 + 2] = 0.0;
	}
}

void collision(void) {
	int i, j;
	double xij, yij, zij;
	double distance, distance2;
	double forceDT; /* forceDT is the impulse of collision between particles */
	double mi, mj;
	double velocity_ix, velocity_iy, velocity_iz;
	double e = COEFFICIENT_OF_RESTITUTION;
	static double VelocityAfterCollision[3 * ARRAY_SIZE];

	for (i = 0; i < 3 * numberOfParticles; i++) {
		VelocityAfterCollision[i] = velocity[i];
	}
	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Fluid) {
			mi = fluidDensity;
			velocity_ix = velocity[i * 3];
			velocity_iy = velocity[i * 3 + 1];
			velocity_iz = velocity[i * 3 + 2];
			for (j = 0; j < numberOfParticles; j++) {
				if ((j == i) || (particleType[j] == ParticleType::Ghost))
					continue;
				xij = position[j * 3] - position[i * 3];
				yij = position[j * 3 + 1] - position[i * 3 + 1];
				zij = position[j * 3 + 2] - position[i * 3 + 2];
				distance2 = (xij * xij) + (yij * yij) + (zij * zij);
				if (distance2 < collisionDistance2) {
					distance = sqrt(distance2);
					forceDT = (velocity_ix - velocity[j * 3]) * (xij / distance) +
					          (velocity_iy - velocity[j * 3 + 1]) * (yij / distance) +
					          (velocity_iz - velocity[j * 3 + 2]) * (zij / distance);
					if (forceDT > 0.0) {
						mj = fluidDensity;
						forceDT *= (1.0 + e) * mi * mj / (mi + mj);
						velocity_ix -= (forceDT / mi) * (xij / distance);
						velocity_iy -= (forceDT / mi) * (yij / distance);
						velocity_iz -= (forceDT / mi) * (zij / distance);
						if (j > i) {
							fprintf(stderr,
							        "WARNING: Collision occurred between %d and "
							        "%d particles.\n",
							        i, j);
						}
					}
				}
			}
			VelocityAfterCollision[i * 3] = velocity_ix;
			VelocityAfterCollision[i * 3 + 1] = velocity_iy;
			VelocityAfterCollision[i * 3 + 2] = velocity_iz;
		}
	}
	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Fluid) {
			position[i * 3] += (VelocityAfterCollision[i * 3] - velocity[i * 3]) * DT;
			position[i * 3 + 1] += (VelocityAfterCollision[i * 3 + 1] - velocity[i * 3 + 1]) * DT;
			position[i * 3 + 2] += (VelocityAfterCollision[i * 3 + 2] - velocity[i * 3 + 2]) * DT;
			velocity[i * 3] = VelocityAfterCollision[i * 3];
			velocity[i * 3 + 1] = VelocityAfterCollision[i * 3 + 1];
			velocity[i * 3 + 2] = VelocityAfterCollision[i * 3 + 2];
		}
	}
}

void calPressure(void) {
	calNumberDensity();
	setBoundaryCondition();
	setSourceTerm();
	setMatrix();
	solveSimultaneousEquationsByGaussEliminationMethod();
	removeNegativePressure();
	setMinimumPressure();
}

void calNumberDensity(void) {
	int i, j;
	double xij, yij, zij;
	double distance, distance2;
	double w;

	for (i = 0; i < numberOfParticles; i++) {
		numberDensity[i] = 0.0;
		if (particleType[i] == ParticleType::Ghost)
			continue;
		for (j = 0; j < numberOfParticles; j++) {
			if ((j == i) || (particleType[j] == ParticleType::Ghost))
				continue;
			xij = position[j * 3] - position[i * 3];
			yij = position[j * 3 + 1] - position[i * 3 + 1];
			zij = position[j * 3 + 2] - position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			w = weight(distance, re_forNumberDensity);
			numberDensity[i] += w;
		}
	}
}

void setBoundaryCondition(void) {
	int i;
	double n0 = n0_forNumberDensity;
	double beta = THRESHOLD_RATIO_OF_NUMBER_DENSITY;

	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Ghost || particleType[i] == ParticleType::DummyWall) {
			boundaryCondition[i] = FluidState::Ignored;
		} else if (numberDensity[i] < beta * n0) {
			boundaryCondition[i] = FluidState::FreeSurface;
		} else {
			boundaryCondition[i] = FluidState::Inner;
		}
	}
}

void setSourceTerm(void) {
	int i;
	double n0 = n0_forNumberDensity;
	double gamma = RELAXATION_COEFFICIENT_FOR_PRESSURE;

	for (i = 0; i < numberOfParticles; i++) {
		sourceTerm[i] = 0.0;
		if (particleType[i] == ParticleType::Ghost || particleType[i] == ParticleType::DummyWall)
			continue;
		if (boundaryCondition[i] == FluidState::Inner) {
			sourceTerm[i] = gamma * (1.0 / (DT * DT)) * ((numberDensity[i] - n0) / n0);
		} else if (boundaryCondition[i] == FluidState::FreeSurface) {
			sourceTerm[i] = 0.0;
		}
	}
}

void setMatrix(void) {
	double xij, yij, zij;
	double distance, distance2;
	double coefficientIJ;
	double n0 = n0_forLaplacian;
	int i, j;
	double a;
	int n = numberOfParticles;

	for (i = 0; i < numberOfParticles; i++) {
		for (j = 0; j < numberOfParticles; j++) {
			coefficientMatrix[i * n + j] = 0.0;
		}
	}

	a = 2.0 * DIM / (n0 * lambda);
	for (i = 0; i < numberOfParticles; i++) {
		if (boundaryCondition[i] != FluidState::Inner)
			continue;
		for (j = 0; j < numberOfParticles; j++) {
			if ((j == i) || (boundaryCondition[j] == FluidState::Ignored))
				continue;
			xij = position[j * 3] - position[i * 3];
			yij = position[j * 3 + 1] - position[i * 3 + 1];
			zij = position[j * 3 + 2] - position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			if (distance >= re_forLaplacian)
				continue;
			coefficientIJ = a * weight(distance, re_forLaplacian) / fluidDensity;
			coefficientMatrix[i * n + j] = (-1.0) * coefficientIJ;
			coefficientMatrix[i * n + i] += coefficientIJ;
		}
		coefficientMatrix[i * n + i] += (COMPRESSIBILITY) / (DT * DT);
	}
	exceptionalProcessingForBoundaryCondition();
}

void exceptionalProcessingForBoundaryCondition(void) {
	/* If there is no Dirichlet boundary condition on the fluid,
	   increase the diagonal terms of the matrix for an exception. This allows
	   us to solve the matrix without Dirichlet boundary conditions. */
	checkBoundaryCondition();
	increaseDiagonalTerm();
}

void checkBoundaryCondition(void) {
	int i, j, count;
	double xij, yij, zij, distance2;

	for (i = 0; i < numberOfParticles; i++) {
		if (boundaryCondition[i] == FluidState::Ignored) {
			flagForCheckingBoundaryCondition[i] = GHOST_OR_DUMMY;
		} else if (boundaryCondition[i] == FluidState::FreeSurface) {
			flagForCheckingBoundaryCondition[i] = DIRICHLET_BOUNDARY_IS_CONNECTED;
		} else {
			flagForCheckingBoundaryCondition[i] = DIRICHLET_BOUNDARY_IS_NOT_CONNECTED;
		}
	}

	do {
		count = 0;
		for (i = 0; i < numberOfParticles; i++) {
			if (flagForCheckingBoundaryCondition[i] == DIRICHLET_BOUNDARY_IS_CONNECTED) {
				for (j = 0; j < numberOfParticles; j++) {
					if (j == i)
						continue;
					if ((particleType[j] == ParticleType::Ghost) || (particleType[j] == ParticleType::DummyWall))
						continue;
					if (flagForCheckingBoundaryCondition[j] == DIRICHLET_BOUNDARY_IS_NOT_CONNECTED) {
						xij = position[j * 3] - position[i * 3];
						yij = position[j * 3 + 1] - position[i * 3 + 1];
						zij = position[j * 3 + 2] - position[i * 3 + 2];
						distance2 = (xij * xij) + (yij * yij) + (zij * zij);
						if (distance2 >= re2_forLaplacian)
							continue;
						flagForCheckingBoundaryCondition[j] = DIRICHLET_BOUNDARY_IS_CONNECTED;
					}
				}
				flagForCheckingBoundaryCondition[i] = DIRICHLET_BOUNDARY_IS_CHECKED;
				count++;
			}
		}
	} while (count != 0); /* This procedure is repeated until the all fluid or wall particles
	                         (which have Dirichlet boundary condition in the particle group)
	                         are in the state of "DIRICHLET_BOUNDARY_IS_CHECKED".*/

	for (i = 0; i < numberOfParticles; i++) {
		if (flagForCheckingBoundaryCondition[i] == DIRICHLET_BOUNDARY_IS_NOT_CONNECTED) {
			fprintf(stderr,
			        "WARNING: There is no dirichlet boundary condition for "
			        "%d-th particle.\n",
			        i);
		}
	}
}

void increaseDiagonalTerm(void) {
	int i;
	int n = numberOfParticles;

	for (i = 0; i < n; i++) {
		if (flagForCheckingBoundaryCondition[i] == DIRICHLET_BOUNDARY_IS_NOT_CONNECTED) {
			coefficientMatrix[i * n + i] = 2.0 * coefficientMatrix[i * n + i];
		}
	}
}

void solveSimultaneousEquationsByGaussEliminationMethod(void) {
	int i, j, k;
	double c;
	double sumOfTerms;
	int n = numberOfParticles;

	for (i = 0; i < n; i++) {
		pressure[i] = 0.0;
	}
	for (i = 0; i < n - 1; i++) {
		if (boundaryCondition[i] != FluidState::Inner)
			continue;
		for (j = i + 1; j < n; j++) {
			if (boundaryCondition[j] == FluidState::Ignored)
				continue;
			c = coefficientMatrix[j * n + i] / coefficientMatrix[i * n + i];
			for (k = i + 1; k < n; k++) {
				coefficientMatrix[j * n + k] -= c * coefficientMatrix[i * n + k];
			}
			sourceTerm[j] -= c * sourceTerm[i];
		}
	}
	for (i = n - 1; i >= 0; i--) {
		if (boundaryCondition[i] != FluidState::Inner)
			continue;
		sumOfTerms = 0.0;
		for (j = i + 1; j < n; j++) {
			if (boundaryCondition[j] == FluidState::Ignored)
				continue;
			sumOfTerms += coefficientMatrix[i * n + j] * pressure[j];
		}
		pressure[i] = (sourceTerm[i] - sumOfTerms) / coefficientMatrix[i * n + i];
	}
}

void removeNegativePressure(void) {
	int i;

	for (i = 0; i < numberOfParticles; i++) {
		if (pressure[i] < 0.0)
			pressure[i] = 0.0;
	}
}

void setMinimumPressure(void) {
	double xij, yij, zij, distance2;
	int i, j;

	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Ghost || particleType[i] == ParticleType::DummyWall)
			continue;
		minimumPressure[i] = pressure[i];
		for (j = 0; j < numberOfParticles; j++) {
			if ((j == i) || (particleType[j] == ParticleType::Ghost))
				continue;
			if (particleType[j] == ParticleType::DummyWall)
				continue;
			xij = position[j * 3] - position[i * 3];
			yij = position[j * 3 + 1] - position[i * 3 + 1];
			zij = position[j * 3 + 2] - position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			if (distance2 >= re2_forGradient)
				continue;
			if (minimumPressure[i] > pressure[j]) {
				minimumPressure[i] = pressure[j];
			}
		}
	}
}

void calPressureGradient(void) {
	int i, j;
	double gradient_x, gradient_y, gradient_z;
	double xij, yij, zij;
	double distance, distance2;
	double w, pij;
	double a;

	a = DIM / n0_forGradient;
	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] != ParticleType::Fluid)
			continue;
		gradient_x = 0.0;
		gradient_y = 0.0;
		gradient_z = 0.0;
		for (j = 0; j < numberOfParticles; j++) {
			if (j == i)
				continue;
			if (particleType[j] == ParticleType::Ghost)
				continue;
			if (particleType[j] == ParticleType::DummyWall)
				continue;
			xij = position[j * 3] - position[i * 3];
			yij = position[j * 3 + 1] - position[i * 3 + 1];
			zij = position[j * 3 + 2] - position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			if (distance < re_forGradient) {
				w = weight(distance, re_forGradient);
				pij = (pressure[j] - minimumPressure[i]) / distance2;
				gradient_x += xij * pij * w;
				gradient_y += yij * pij * w;
				gradient_z += zij * pij * w;
			}
		}
		gradient_x *= a;
		gradient_y *= a;
		gradient_z *= a;
		acceleration[i * 3] = (-1.0) * gradient_x / fluidDensity;
		acceleration[i * 3 + 1] = (-1.0) * gradient_y / fluidDensity;
		acceleration[i * 3 + 2] = (-1.0) * gradient_z / fluidDensity;
	}
}

void moveParticleUsingPressureGradient(void) {
	int i;

	for (i = 0; i < numberOfParticles; i++) {
		if (particleType[i] == ParticleType::Fluid) {
			velocity[i * 3] += acceleration[i * 3] * DT;
			velocity[i * 3 + 1] += acceleration[i * 3 + 1] * DT;
			velocity[i * 3 + 2] += acceleration[i * 3 + 2] * DT;

			position[i * 3] += acceleration[i * 3] * DT * DT;
			position[i * 3 + 1] += acceleration[i * 3 + 1] * DT * DT;
			position[i * 3 + 2] += acceleration[i * 3 + 2] * DT * DT;
		}
		acceleration[i * 3] = 0.0;
		acceleration[i * 3 + 1] = 0.0;
		acceleration[i * 3 + 2] = 0.0;
	}
}

void writeData_inProfFormat(void) {
	int i;
	FILE* fp;
	char fileName[256];

	sprintf(fileName, "output_%04d.prof", fileNumber);
	fp = fopen(fileName, "w");
	fprintf(fp, "%lf\n", Time);
	fprintf(fp, "%d\n", numberOfParticles);
	for (i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "%d %lf %lf %lf %lf %lf %lf %lf %lf\n", particleType[i], position[i * 3], position[i * 3 + 1],
		        position[i * 3 + 2], velocity[i * 3], velocity[i * 3 + 1], velocity[i * 3 + 2], pressure[i],
		        numberDensity[i]);
	}
	fclose(fp);
	fileNumber++;
}

void writeData_inVtuFormat(void) {
	double absoluteValueOfVelocity;
	FILE* fp;
	char fileName[1024];

	sprintf(fileName, "particle_%04d.vtu", fileNumber);
	fp = fopen(fileName, "w");
	fprintf(fp, "<?xml version='1.0' encoding='UTF-8'?>\n");
	fprintf(fp, "<VTKFile xmlns='VTK' byte_order='LittleEndian' version='0.1' "
	            "type='UnstructuredGrid'>\n");
	fprintf(fp, "<UnstructuredGrid>\n");
	fprintf(fp, "<Piece NumberOfCells='%d' NumberOfPoints='%d'>\n", numberOfParticles, numberOfParticles);
	fprintf(fp, "<Points>\n");
	fprintf(fp, "<DataArray NumberOfComponents='3' type='Float32' "
	            "Name='position' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "%lf %lf %lf\n", position[i * 3], position[i * 3 + 1], position[i * 3 + 2]);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "</Points>\n");
	fprintf(fp, "<PointData>\n");
	fprintf(fp, "<DataArray NumberOfComponents='1' type='Int32' "
	            "Name='particleType' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "%d\n", particleType[i]);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray NumberOfComponents='1' type='Float32' "
	            "Name='velocity' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		absoluteValueOfVelocity = sqrt(velocity[i * 3] * velocity[i * 3] + velocity[i * 3 + 1] * velocity[i * 3 + 1] +
		                               velocity[i * 3 + 2] * velocity[i * 3 + 2]);
		fprintf(fp, "%f\n", (float)absoluteValueOfVelocity);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray NumberOfComponents='1' type='Float32' "
	            "Name='pressure' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "%f\n", (float)pressure[i]);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "</PointData>\n");
	fprintf(fp, "<Cells>\n");
	fprintf(fp, "<DataArray type='Int32' Name='connectivity' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "%d\n", i);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray type='Int32' Name='offsets' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "%d\n", i + 1);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray type='UInt8' Name='types' format='ascii'>\n");
	for (int i = 0; i < numberOfParticles; i++) {
		fprintf(fp, "1\n");
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "</Cells>\n");
	fprintf(fp, "</Piece>\n");
	fprintf(fp, "</UnstructuredGrid>\n");
	fprintf(fp, "</VTKFile>\n");
	fclose(fp);
}
