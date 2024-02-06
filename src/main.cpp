#include "mps.cpp"
#include "particle.hpp"
#include "settings.hpp"
#include "yaml-cpp/yaml.h"
#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

void readData(Settings& settings, vector<Particle>& particles, double& initialTIme);

int main(int argc, char** argv) {
	Settings settings;
	vector<Particle> particles;
	double initialTime;
	readData(settings, particles, initialTime);

	MPS mps = MPS(settings, particles, initialTime);
	mps.run();

	return 0;
}

void readData(Settings& settings, vector<Particle>& particles, double& initialTime) {
	std::stringstream ss;
	std::ifstream ifs;

	// read input.yml
	// TODO: input file name should be given as an argument
	YAML::Node input = YAML::LoadFile("./input/input.yml");
	settings.load(input);

	// read input.prof
	ss.str("./input/input.prof");
	ifs.open(ss.str());
	if (ifs.fail()) {
		std::cerr << "cannot read " << ss.str() << std::endl;
		std::exit(-1);
	}

	int particleSize;
	ifs >> initialTime;
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
}