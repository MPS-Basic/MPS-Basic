#include <string.h>

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

// particle type
constexpr int GHOST = -1;
constexpr int FLUID = 0;
constexpr int WALL = 2;
constexpr int DUMMY_WALL = 3;

// free surface detection
constexpr double THRESHOLD_RATIO_OF_NUMBER_DENSITY = 0.97;
constexpr int GHOST_OR_DUMMY = -1;
constexpr int SURFACE_PARTICLE = 1;
constexpr int INNER_PARTICLE = 0;

// boundary condition
constexpr int DIRICHLET_BOUNDARY_IS_NOT_CONNECTED = 0;
constexpr int DIRICHLET_BOUNDARY_IS_CONNECTED = 1;
constexpr int DIRICHLET_BOUNDARY_IS_CHECKED = 2;

// partile collision
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

enum class BoundaryState {
	Ignored,
	Surface,
	Inner,
};

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
void solveSimultaniousEquationsByGaussEliminationMethod(void);
void removeNegativePressure(void);
void setMinimumPressure(void);
void calPressureGradient(void);
void moveParticleUsingPressureGradient(void);
void writeData_inProfFormat(void);
void writeData_inVtuFormat(void);

static double Acceleration[3 * ARRAY_SIZE];
static int ParticleType[ARRAY_SIZE];
static double Position[3 * ARRAY_SIZE];
static double Velocity[3 * ARRAY_SIZE];
static double Pressure[ARRAY_SIZE];
static double NumberDensity[ARRAY_SIZE];
static int BoundaryCondition[ARRAY_SIZE];
static double SourceTerm[ARRAY_SIZE];
static int FlagForCheckingBoundaryCondition[ARRAY_SIZE];
static double CoefficientMatrix[ARRAY_SIZE * ARRAY_SIZE];
static double MinimumPressure[ARRAY_SIZE];
int FileNumber;
double Time;
int NumberOfParticles;
double Re_forNumberDensity, Re2_forNumberDensity;
double Re_forGradient, Re2_forGradient;
double Re_forLaplacian, Re2_forLaplacian;
double N0_forNumberDensity;
double N0_forGradient;
double N0_forLaplacian;
double Lambda;
double collisionDistance, collisionDistance2;
double FluidDensity;

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
			if (((x > -4.0 * PARTICLE_DISTANCE + EPS) &&
			     (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) &&
			     (y <= 0.6 + EPS))) {
				ParticleType[i] = DUMMY_WALL;
				flagOfParticleGeneration = ON;
			}

			/* wall region */
			if (((x > -2.0 * PARTICLE_DISTANCE + EPS) &&
			     (x <= 1.00 + 2.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) &&
			     (y <= 0.6 + EPS))) {
				ParticleType[i] = WALL;
				flagOfParticleGeneration = ON;
			}

			/* wall region */
			if (((x > -4.0 * PARTICLE_DISTANCE + EPS) &&
			     (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
			    ((y > 0.6 - 2.0 * PARTICLE_DISTANCE + EPS) &&
			     (y <= 0.6 + EPS))) {
				ParticleType[i] = WALL;
				flagOfParticleGeneration = ON;
			}

			/* empty region */
			if (((x > 0.0 + EPS) && (x <= 1.00 + EPS)) && (y > 0.0 + EPS)) {
				flagOfParticleGeneration = OFF;
			}

			/* fluid region */
			if (((x > 0.0 + EPS) && (x <= 0.25 + EPS)) &&
			    ((y > 0.0 + EPS) && (y <= 0.50 + EPS))) {
				ParticleType[i] = FLUID;
				flagOfParticleGeneration = ON;
			}

			if (flagOfParticleGeneration == ON) {
				Position[i * 3] = x;
				Position[i * 3 + 1] = y;
				Position[i * 3 + 2] = z;
				i++;
			}
		}
	}
	NumberOfParticles = i;
	for (i = 0; i < NumberOfParticles * 3; i++) {
		Velocity[i] = 0.0;
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
				if ((((x > -4.0 * PARTICLE_DISTANCE + EPS) &&
				      (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) &&
				      (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) &&
				     (z <= 0.3 + 4.0 * PARTICLE_DISTANCE + EPS))) {
					ParticleType[i] = DUMMY_WALL;
					flagOfParticleGeneration = ON;
				}

				/* wall region */
				if ((((x > -2.0 * PARTICLE_DISTANCE + EPS) &&
				      (x <= 1.00 + 2.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) &&
				      (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 2.0 * PARTICLE_DISTANCE + EPS) &&
				     (z <= 0.3 + 2.0 * PARTICLE_DISTANCE + EPS))) {
					ParticleType[i] = WALL;
					flagOfParticleGeneration = ON;
				}

				/* wall region */
				if ((((x > -4.0 * PARTICLE_DISTANCE + EPS) &&
				      (x <= 1.00 + 4.0 * PARTICLE_DISTANCE + EPS)) &&
				     ((y > 0.6 - 2.0 * PARTICLE_DISTANCE + EPS) &&
				      (y <= 0.6 + EPS))) &&
				    ((z > 0.0 - 4.0 * PARTICLE_DISTANCE + EPS) &&
				     (z <= 0.3 + 4.0 * PARTICLE_DISTANCE + EPS))) {
					ParticleType[i] = WALL;
					flagOfParticleGeneration = ON;
				}

				/* empty region */
				if ((((x > 0.0 + EPS) && (x <= 1.00 + EPS)) &&
				     (y > 0.0 + EPS)) &&
				    ((z > 0.0 + EPS) && (z <= 0.3 + EPS))) {
					flagOfParticleGeneration = OFF;
				}

				/* fluid region */
				if ((((x > 0.0 + EPS) && (x <= 0.25 + EPS)) &&
				     ((y > 0.0 + EPS) && (y < 0.5 + EPS))) &&
				    ((z > 0.0 + EPS) && (z <= 0.3 + EPS))) {
					ParticleType[i] = FLUID;
					flagOfParticleGeneration = ON;
				}

				if (flagOfParticleGeneration == ON) {
					Position[i * 3] = x;
					Position[i * 3 + 1] = y;
					Position[i * 3 + 2] = z;
					i++;
				}
			}
		}
	}
	NumberOfParticles = i;
	for (i = 0; i < NumberOfParticles * 3; i++) {
		Velocity[i] = 0.0;
	}
}

void calConstantParameter(void) {
	Re_forNumberDensity = RADIUS_FOR_NUMBER_DENSITY;
	Re_forGradient = RADIUS_FOR_GRADIENT;
	Re_forLaplacian = RADIUS_FOR_LAPLACIAN;
	Re2_forNumberDensity = Re_forNumberDensity * Re_forNumberDensity;
	Re2_forGradient = Re_forGradient * Re_forGradient;
	Re2_forLaplacian = Re_forLaplacian * Re_forLaplacian;
	calNZeroAndLambda();
	FluidDensity = FLUID_DENSITY;
	collisionDistance = COLLISION_DISTANCE;
	collisionDistance2 = collisionDistance * collisionDistance;
	FileNumber = 0;
	Time = 0.0;
}

void calNZeroAndLambda(void) {
	int iX, iY, iZ;
	int iZ_start, iZ_end;
	double xj, yj, zj, distance, distance2;
	double xi, yi, zi;

	if (DIM == 2) {
		iZ_start = 0;
		iZ_end = 1;
	} else {
		iZ_start = -4;
		iZ_end = 5;
	}

	N0_forNumberDensity = 0.0;
	N0_forGradient = 0.0;
	N0_forLaplacian = 0.0;
	Lambda = 0.0;
	xi = 0.0;
	yi = 0.0;
	zi = 0.0;

	for (iX = -4; iX < 5; iX++) {
		for (iY = -4; iY < 5; iY++) {
			for (iZ = iZ_start; iZ < iZ_end; iZ++) {
				if (((iX == 0) && (iY == 0)) && (iZ == 0))
					continue;
				xj = PARTICLE_DISTANCE * (double)(iX);
				yj = PARTICLE_DISTANCE * (double)(iY);
				zj = PARTICLE_DISTANCE * (double)(iZ);
				distance2 = (xj - xi) * (xj - xi) + (yj - yi) * (yj - yi) +
				            (zj - zi) * (zj - zi);
				distance = sqrt(distance2);
				N0_forNumberDensity += weight(distance, Re_forNumberDensity);
				N0_forGradient += weight(distance, Re_forGradient);
				N0_forLaplacian += weight(distance, Re_forLaplacian);
				Lambda += distance2 * weight(distance, Re_forLaplacian);
			}
		}
	}
	Lambda = Lambda / N0_forLaplacian;

	fprintf(stdout, "%lf", N0_forNumberDensity);
}

double weight(double distance, double re) {
	double weightIJ;

	if (distance >= re) {
		weightIJ = 0.0;
	} else {
		weightIJ = (re / distance) - 1.0;
	}
	return weightIJ;
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
			printf(
			    "TimeStepNumber: %4d   Time: %lf(s)   NumberOfParticless: %d\n",
			    iTimeStep, Time, NumberOfParticles);
			writeData_inVtuFormat();
			writeData_inProfFormat();
		}
		if (Time >= FINISH_TIME) {
			break;
		}
	}
}

void calGravity(void) {
	int i;

	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == FLUID) {
			Acceleration[i * 3] = G_X;
			Acceleration[i * 3 + 1] = G_Y;
			Acceleration[i * 3 + 2] = G_Z;
		} else {
			Acceleration[i * 3] = 0.0;
			Acceleration[i * 3 + 1] = 0.0;
			Acceleration[i * 3 + 2] = 0.0;
		}
	}
}

void calViscosity(void) {
	int i, j;
	double viscosityTerm_x, viscosityTerm_y, viscosityTerm_z;
	double distance, distance2;
	double w;
	double xij, yij, zij;
	double a;

	a = (KINEMATIC_VISCOSITY) * (2.0 * DIM) / (N0_forLaplacian * Lambda);
	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] != FLUID)
			continue;
		viscosityTerm_x = 0.0;
		viscosityTerm_y = 0.0;
		viscosityTerm_z = 0.0;

		for (j = 0; j < NumberOfParticles; j++) {
			if ((j == i) || (ParticleType[j] == GHOST))
				continue;
			xij = Position[j * 3] - Position[i * 3];
			yij = Position[j * 3 + 1] - Position[i * 3 + 1];
			zij = Position[j * 3 + 2] - Position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			if (distance < Re_forLaplacian) {
				w = weight(distance, Re_forLaplacian);
				viscosityTerm_x += (Velocity[j * 3] - Velocity[i * 3]) * w;
				viscosityTerm_y +=
				    (Velocity[j * 3 + 1] - Velocity[i * 3 + 1]) * w;
				viscosityTerm_z +=
				    (Velocity[j * 3 + 2] - Velocity[i * 3 + 2]) * w;
			}
		}
		viscosityTerm_x = viscosityTerm_x * a;
		viscosityTerm_y = viscosityTerm_y * a;
		viscosityTerm_z = viscosityTerm_z * a;
		Acceleration[i * 3] += viscosityTerm_x;
		Acceleration[i * 3 + 1] += viscosityTerm_y;
		Acceleration[i * 3 + 2] += viscosityTerm_z;
	}
}

void moveParticle(void) {
	int i;

	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == FLUID) {
			Velocity[i * 3] += Acceleration[i * 3] * DT;
			Velocity[i * 3 + 1] += Acceleration[i * 3 + 1] * DT;
			Velocity[i * 3 + 2] += Acceleration[i * 3 + 2] * DT;

			Position[i * 3] += Velocity[i * 3] * DT;
			Position[i * 3 + 1] += Velocity[i * 3 + 1] * DT;
			Position[i * 3 + 2] += Velocity[i * 3 + 2] * DT;
		}
		Acceleration[i * 3] = 0.0;
		Acceleration[i * 3 + 1] = 0.0;
		Acceleration[i * 3 + 2] = 0.0;
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

	for (i = 0; i < 3 * NumberOfParticles; i++) {
		VelocityAfterCollision[i] = Velocity[i];
	}
	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == FLUID) {
			mi = FluidDensity;
			velocity_ix = Velocity[i * 3];
			velocity_iy = Velocity[i * 3 + 1];
			velocity_iz = Velocity[i * 3 + 2];
			for (j = 0; j < NumberOfParticles; j++) {
				if ((j == i) || (ParticleType[j] == GHOST))
					continue;
				xij = Position[j * 3] - Position[i * 3];
				yij = Position[j * 3 + 1] - Position[i * 3 + 1];
				zij = Position[j * 3 + 2] - Position[i * 3 + 2];
				distance2 = (xij * xij) + (yij * yij) + (zij * zij);
				if (distance2 < collisionDistance2) {
					distance = sqrt(distance2);
					forceDT =
					    (velocity_ix - Velocity[j * 3]) * (xij / distance) +
					    (velocity_iy - Velocity[j * 3 + 1]) * (yij / distance) +
					    (velocity_iz - Velocity[j * 3 + 2]) * (zij / distance);
					if (forceDT > 0.0) {
						mj = FluidDensity;
						forceDT *= (1.0 + e) * mi * mj / (mi + mj);
						velocity_ix -= (forceDT / mi) * (xij / distance);
						velocity_iy -= (forceDT / mi) * (yij / distance);
						velocity_iz -= (forceDT / mi) * (zij / distance);
						if (j > i) {
							fprintf(stderr,
							        "WARNING: Collision occured between %d and "
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
	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == FLUID) {
			Position[i * 3] +=
			    (VelocityAfterCollision[i * 3] - Velocity[i * 3]) * DT;
			Position[i * 3 + 1] +=
			    (VelocityAfterCollision[i * 3 + 1] - Velocity[i * 3 + 1]) * DT;
			Position[i * 3 + 2] +=
			    (VelocityAfterCollision[i * 3 + 2] - Velocity[i * 3 + 2]) * DT;
			Velocity[i * 3] = VelocityAfterCollision[i * 3];
			Velocity[i * 3 + 1] = VelocityAfterCollision[i * 3 + 1];
			Velocity[i * 3 + 2] = VelocityAfterCollision[i * 3 + 2];
		}
	}
}

void calPressure(void) {
	calNumberDensity();
	setBoundaryCondition();
	setSourceTerm();
	setMatrix();
	solveSimultaniousEquationsByGaussEliminationMethod();
	removeNegativePressure();
	setMinimumPressure();
}

void calNumberDensity(void) {
	int i, j;
	double xij, yij, zij;
	double distance, distance2;
	double w;

	for (i = 0; i < NumberOfParticles; i++) {
		NumberDensity[i] = 0.0;
		if (ParticleType[i] == GHOST)
			continue;
		for (j = 0; j < NumberOfParticles; j++) {
			if ((j == i) || (ParticleType[j] == GHOST))
				continue;
			xij = Position[j * 3] - Position[i * 3];
			yij = Position[j * 3 + 1] - Position[i * 3 + 1];
			zij = Position[j * 3 + 2] - Position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			w = weight(distance, Re_forNumberDensity);
			NumberDensity[i] += w;
		}
	}
}

void setBoundaryCondition(void) {
	int i;
	double n0 = N0_forNumberDensity;
	double beta = THRESHOLD_RATIO_OF_NUMBER_DENSITY;

	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == GHOST || ParticleType[i] == DUMMY_WALL) {
			BoundaryCondition[i] = GHOST_OR_DUMMY;
		} else if (NumberDensity[i] < beta * n0) {
			BoundaryCondition[i] = SURFACE_PARTICLE;
		} else {
			BoundaryCondition[i] = INNER_PARTICLE;
		}
	}
}

void setSourceTerm(void) {
	int i;
	double n0 = N0_forNumberDensity;
	double gamma = RELAXATION_COEFFICIENT_FOR_PRESSURE;

	for (i = 0; i < NumberOfParticles; i++) {
		SourceTerm[i] = 0.0;
		if (ParticleType[i] == GHOST || ParticleType[i] == DUMMY_WALL)
			continue;
		if (BoundaryCondition[i] == INNER_PARTICLE) {
			SourceTerm[i] =
			    gamma * (1.0 / (DT * DT)) * ((NumberDensity[i] - n0) / n0);
		} else if (BoundaryCondition[i] == SURFACE_PARTICLE) {
			SourceTerm[i] = 0.0;
		}
	}
}

void setMatrix(void) {
	double xij, yij, zij;
	double distance, distance2;
	double coefficientIJ;
	double n0 = N0_forLaplacian;
	int i, j;
	double a;
	int n = NumberOfParticles;

	for (i = 0; i < NumberOfParticles; i++) {
		for (j = 0; j < NumberOfParticles; j++) {
			CoefficientMatrix[i * n + j] = 0.0;
		}
	}

	a = 2.0 * DIM / (n0 * Lambda);
	for (i = 0; i < NumberOfParticles; i++) {
		if (BoundaryCondition[i] != INNER_PARTICLE)
			continue;
		for (j = 0; j < NumberOfParticles; j++) {
			if ((j == i) || (BoundaryCondition[j] == GHOST_OR_DUMMY))
				continue;
			xij = Position[j * 3] - Position[i * 3];
			yij = Position[j * 3 + 1] - Position[i * 3 + 1];
			zij = Position[j * 3 + 2] - Position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			if (distance >= Re_forLaplacian)
				continue;
			coefficientIJ =
			    a * weight(distance, Re_forLaplacian) / FluidDensity;
			CoefficientMatrix[i * n + j] = (-1.0) * coefficientIJ;
			CoefficientMatrix[i * n + i] += coefficientIJ;
		}
		CoefficientMatrix[i * n + i] += (COMPRESSIBILITY) / (DT * DT);
	}
	exceptionalProcessingForBoundaryCondition();
}

void exceptionalProcessingForBoundaryCondition(void) {
	/* If tere is no Dirichlet boundary condition on the fluid,
	   increase the diagonal terms of the matrix for an exception. This allows
	   us to solve the matrix without Dirichlet boundary conditions. */
	checkBoundaryCondition();
	increaseDiagonalTerm();
}

void checkBoundaryCondition(void) {
	int i, j, count;
	double xij, yij, zij, distance2;

	for (i = 0; i < NumberOfParticles; i++) {
		if (BoundaryCondition[i] == GHOST_OR_DUMMY) {
			FlagForCheckingBoundaryCondition[i] = GHOST_OR_DUMMY;
		} else if (BoundaryCondition[i] == SURFACE_PARTICLE) {
			FlagForCheckingBoundaryCondition[i] =
			    DIRICHLET_BOUNDARY_IS_CONNECTED;
		} else {
			FlagForCheckingBoundaryCondition[i] =
			    DIRICHLET_BOUNDARY_IS_NOT_CONNECTED;
		}
	}

	do {
		count = 0;
		for (i = 0; i < NumberOfParticles; i++) {
			if (FlagForCheckingBoundaryCondition[i] ==
			    DIRICHLET_BOUNDARY_IS_CONNECTED) {
				for (j = 0; j < NumberOfParticles; j++) {
					if (j == i)
						continue;
					if ((ParticleType[j] == GHOST) ||
					    (ParticleType[j] == DUMMY_WALL))
						continue;
					if (FlagForCheckingBoundaryCondition[j] ==
					    DIRICHLET_BOUNDARY_IS_NOT_CONNECTED) {
						xij = Position[j * 3] - Position[i * 3];
						yij = Position[j * 3 + 1] - Position[i * 3 + 1];
						zij = Position[j * 3 + 2] - Position[i * 3 + 2];
						distance2 = (xij * xij) + (yij * yij) + (zij * zij);
						if (distance2 >= Re2_forLaplacian)
							continue;
						FlagForCheckingBoundaryCondition[j] =
						    DIRICHLET_BOUNDARY_IS_CONNECTED;
					}
				}
				FlagForCheckingBoundaryCondition[i] =
				    DIRICHLET_BOUNDARY_IS_CHECKED;
				count++;
			}
		}
	} while (
	    count !=
	    0); /* This procedure is repeated until the all fluid or wall particles
	           (which have Dirhchlet boundary condition in the particle group)
	           are in the state of "DIRICHLET_BOUNDARY_IS_CHECKED".*/

	for (i = 0; i < NumberOfParticles; i++) {
		if (FlagForCheckingBoundaryCondition[i] ==
		    DIRICHLET_BOUNDARY_IS_NOT_CONNECTED) {
			fprintf(stderr,
			        "WARNING: There is no dirichlet boundary condition for "
			        "%d-th particle.\n",
			        i);
		}
	}
}

void increaseDiagonalTerm(void) {
	int i;
	int n = NumberOfParticles;

	for (i = 0; i < n; i++) {
		if (FlagForCheckingBoundaryCondition[i] ==
		    DIRICHLET_BOUNDARY_IS_NOT_CONNECTED) {
			CoefficientMatrix[i * n + i] = 2.0 * CoefficientMatrix[i * n + i];
		}
	}
}

void solveSimultaniousEquationsByGaussEliminationMethod(void) {
	int i, j, k;
	double c;
	double sumOfTerms;
	int n = NumberOfParticles;

	for (i = 0; i < n; i++) {
		Pressure[i] = 0.0;
	}
	for (i = 0; i < n - 1; i++) {
		if (BoundaryCondition[i] != INNER_PARTICLE)
			continue;
		for (j = i + 1; j < n; j++) {
			if (BoundaryCondition[j] == GHOST_OR_DUMMY)
				continue;
			c = CoefficientMatrix[j * n + i] / CoefficientMatrix[i * n + i];
			for (k = i + 1; k < n; k++) {
				CoefficientMatrix[j * n + k] -=
				    c * CoefficientMatrix[i * n + k];
			}
			SourceTerm[j] -= c * SourceTerm[i];
		}
	}
	for (i = n - 1; i >= 0; i--) {
		if (BoundaryCondition[i] != INNER_PARTICLE)
			continue;
		sumOfTerms = 0.0;
		for (j = i + 1; j < n; j++) {
			if (BoundaryCondition[j] == GHOST_OR_DUMMY)
				continue;
			sumOfTerms += CoefficientMatrix[i * n + j] * Pressure[j];
		}
		Pressure[i] =
		    (SourceTerm[i] - sumOfTerms) / CoefficientMatrix[i * n + i];
	}
}

void removeNegativePressure(void) {
	int i;

	for (i = 0; i < NumberOfParticles; i++) {
		if (Pressure[i] < 0.0)
			Pressure[i] = 0.0;
	}
}

void setMinimumPressure(void) {
	double xij, yij, zij, distance2;
	int i, j;

	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == GHOST || ParticleType[i] == DUMMY_WALL)
			continue;
		MinimumPressure[i] = Pressure[i];
		for (j = 0; j < NumberOfParticles; j++) {
			if ((j == i) || (ParticleType[j] == GHOST))
				continue;
			if (ParticleType[j] == DUMMY_WALL)
				continue;
			xij = Position[j * 3] - Position[i * 3];
			yij = Position[j * 3 + 1] - Position[i * 3 + 1];
			zij = Position[j * 3 + 2] - Position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			if (distance2 >= Re2_forGradient)
				continue;
			if (MinimumPressure[i] > Pressure[j]) {
				MinimumPressure[i] = Pressure[j];
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

	a = DIM / N0_forGradient;
	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] != FLUID)
			continue;
		gradient_x = 0.0;
		gradient_y = 0.0;
		gradient_z = 0.0;
		for (j = 0; j < NumberOfParticles; j++) {
			if (j == i)
				continue;
			if (ParticleType[j] == GHOST)
				continue;
			if (ParticleType[j] == DUMMY_WALL)
				continue;
			xij = Position[j * 3] - Position[i * 3];
			yij = Position[j * 3 + 1] - Position[i * 3 + 1];
			zij = Position[j * 3 + 2] - Position[i * 3 + 2];
			distance2 = (xij * xij) + (yij * yij) + (zij * zij);
			distance = sqrt(distance2);
			if (distance < Re_forGradient) {
				w = weight(distance, Re_forGradient);
				pij = (Pressure[j] - MinimumPressure[i]) / distance2;
				gradient_x += xij * pij * w;
				gradient_y += yij * pij * w;
				gradient_z += zij * pij * w;
			}
		}
		gradient_x *= a;
		gradient_y *= a;
		gradient_z *= a;
		Acceleration[i * 3] = (-1.0) * gradient_x / FluidDensity;
		Acceleration[i * 3 + 1] = (-1.0) * gradient_y / FluidDensity;
		Acceleration[i * 3 + 2] = (-1.0) * gradient_z / FluidDensity;
	}
}

void moveParticleUsingPressureGradient(void) {
	int i;

	for (i = 0; i < NumberOfParticles; i++) {
		if (ParticleType[i] == FLUID) {
			Velocity[i * 3] += Acceleration[i * 3] * DT;
			Velocity[i * 3 + 1] += Acceleration[i * 3 + 1] * DT;
			Velocity[i * 3 + 2] += Acceleration[i * 3 + 2] * DT;

			Position[i * 3] += Acceleration[i * 3] * DT * DT;
			Position[i * 3 + 1] += Acceleration[i * 3 + 1] * DT * DT;
			Position[i * 3 + 2] += Acceleration[i * 3 + 2] * DT * DT;
		}
		Acceleration[i * 3] = 0.0;
		Acceleration[i * 3 + 1] = 0.0;
		Acceleration[i * 3 + 2] = 0.0;
	}
}

void writeData_inProfFormat(void) {
	int i;
	FILE* fp;
	char fileName[256];

	sprintf(fileName, "output_%04d.prof", FileNumber);
	fp = fopen(fileName, "w");
	fprintf(fp, "%lf\n", Time);
	fprintf(fp, "%d\n", NumberOfParticles);
	for (i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "%d %lf %lf %lf %lf %lf %lf %lf %lf\n", ParticleType[i],
		        Position[i * 3], Position[i * 3 + 1], Position[i * 3 + 2],
		        Velocity[i * 3], Velocity[i * 3 + 1], Velocity[i * 3 + 2],
		        Pressure[i], NumberDensity[i]);
	}
	fclose(fp);
	FileNumber++;
}

void writeData_inVtuFormat(void) {
	double absoluteValueOfVelocity;
	FILE* fp;
	char fileName[1024];

	sprintf(fileName, "particle_%04d.vtu", FileNumber);
	fp = fopen(fileName, "w");
	fprintf(fp, "<?xml version='1.0' encoding='UTF-8'?>\n");
	fprintf(fp, "<VTKFile xmlns='VTK' byte_order='LittleEndian' version='0.1' "
	            "type='UnstructuredGrid'>\n");
	fprintf(fp, "<UnstructuredGrid>\n");
	fprintf(fp, "<Piece NumberOfCells='%d' NumberOfPoints='%d'>\n",
	        NumberOfParticles, NumberOfParticles);
	fprintf(fp, "<Points>\n");
	fprintf(fp, "<DataArray NumberOfComponents='3' type='Float32' "
	            "Name='Position' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "%lf %lf %lf\n", Position[i * 3], Position[i * 3 + 1],
		        Position[i * 3 + 2]);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "</Points>\n");
	fprintf(fp, "<PointData>\n");
	fprintf(fp, "<DataArray NumberOfComponents='1' type='Int32' "
	            "Name='ParticleType' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "%d\n", ParticleType[i]);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray NumberOfComponents='1' type='Float32' "
	            "Name='Velocity' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		absoluteValueOfVelocity =
		    sqrt(Velocity[i * 3] * Velocity[i * 3] +
		         Velocity[i * 3 + 1] * Velocity[i * 3 + 1] +
		         Velocity[i * 3 + 2] * Velocity[i * 3 + 2]);
		fprintf(fp, "%f\n", (float)absoluteValueOfVelocity);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray NumberOfComponents='1' type='Float32' "
	            "Name='pressure' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "%f\n", (float)Pressure[i]);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "</PointData>\n");
	fprintf(fp, "<Cells>\n");
	fprintf(fp,
	        "<DataArray type='Int32' Name='connectivity' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "%d\n", i);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray type='Int32' Name='offsets' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "%d\n", i + 1);
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "<DataArray type='UInt8' Name='types' format='ascii'>\n");
	for (int i = 0; i < NumberOfParticles; i++) {
		fprintf(fp, "1\n");
	}
	fprintf(fp, "</DataArray>\n");
	fprintf(fp, "</Cells>\n");
	fprintf(fp, "</Piece>\n");
	fprintf(fp, "</UnstructuredGrid>\n");
	fprintf(fp, "</VTKFile>\n");
	fclose(fp);
}
