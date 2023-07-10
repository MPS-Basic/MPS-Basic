#pragma once
#include "Eigen/Dense"

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
	Eigen::Vector3d acceleration = Eigen::Vector3d::Zero();
	double pressure              = 0;
	double numberDensity         = 0;
	FluidState boundaryCondition = FluidState::Ignored;
	double sourceTerm            = 0;
	double minimumPressure       = 0;

	Particle(ParticleType type, Eigen::Vector3d pos, Eigen::Vector3d vel){
		this->particleType = type;
		this->position = pos;
		this->velocity = vel;
	}

	double inverseDensity(double& density) const {
		switch (particleType) {
		case ParticleType::Ghost:
			return std::numeric_limits<double>::infinity();

		case ParticleType::Fluid:
			return 1 / density;

		case ParticleType::Wall:
		case ParticleType::DummyWall:
		default:
			return 0;
		}
	}
};
