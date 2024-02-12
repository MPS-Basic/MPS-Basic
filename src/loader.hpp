#pragma once

#include "input.hpp"
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>


class Loader {
public:
	Input load(const std::filesystem::path& settingPath) {
		Input input;

		YAML::Node settingYaml = YAML::LoadFile(settingPath);
		input.settings = loadSettingYaml(settingYaml);

		auto [initialTime, particles] = loadParticleProf(input.settings.profPath);
		input.initialTime = initialTime;
		input.particles = particles;

		return input;
	}

private:
	Settings loadSettingYaml(const YAML::Node& yaml) {
		Settings s;

		// computational conditions
		s.dim              = yaml["dim"].as<int>();
		s.particleDistance = yaml["particleDistance"].as<double>();
		s.dt               = yaml["dt"].as<double>();
		s.finishTime       = yaml["finishTime"].as<double>();
		s.outputPeriod     = yaml["outputPeriod"].as<double>();
		s.cflCondition     = yaml["cflCondition"].as<double>();
		s.numPhysicalCores = yaml["numPhysicalCores"].as<int>();

		// physical properties
		s.fluidDensity       = yaml["fluidDensity"].as<double>();
		s.kinematicViscosity = yaml["kinematicViscosity"].as<double>();

		// gravity
		s.gravity[0] = yaml["gravity"][0].as<double>();
		s.gravity[1] = yaml["gravity"][1].as<double>();
		s.gravity[2] = yaml["gravity"][2].as<double>();

		// free surface detection
		s.surfaceDetectionRatio = yaml["surfaceDetectionRatio"].as<double>();

		// pressure Poisson equation
		s.compressibility                  = yaml["compressibility"].as<double>();
		s.relaxationCoefficientForPressure = yaml["relaxationCoefficientForPressure"].as<double>();

		// collision
		s.collisionDistance        = yaml["collisionDistanceRatio"].as<double>() * s.particleDistance;
		s.coefficientOfRestitution = yaml["coefficientOfRestitution"].as<double>();

		// effective radius
		s.re_forNumberDensity = yaml["radiusRatioForNumberDensity"].as<double>() * s.particleDistance;
		s.re_forGradient      = yaml["radiusRatioForGradient"].as<double>() * s.particleDistance;
		s.re_forLaplacian     = yaml["radiusRatioForLaplacian"].as<double>() * s.particleDistance;
		s.reMax               = std::max({s.re_forNumberDensity, s.re_forGradient, s.re_forLaplacian});

		// domain
		s.domain.xMin    = yaml["domainMin"][0].as<double>();
		s.domain.xMax    = yaml["domainMax"][0].as<double>();
		s.domain.yMin    = yaml["domainMin"][1].as<double>();
		s.domain.yMax    = yaml["domainMax"][1].as<double>();
		s.domain.zMin    = yaml["domainMin"][2].as<double>();
		s.domain.zMax    = yaml["domainMax"][2].as<double>();
		s.domain.xLength = s.domain.xMax - s.domain.xMin;
		s.domain.yLength = s.domain.yMax - s.domain.yMin;
		s.domain.zLength = s.domain.zMax - s.domain.zMin;

		// profpath
		s.profPath = std::filesystem::path(yaml["profPath"].as<std::string>());

		return s;
	}

	std::pair<double, std::vector<Particle>> loadParticleProf(const std::filesystem::path& profPath) {
		std::ifstream ifs;
		ifs.open(profPath);
		if (ifs.fail()) {
			std::cerr << "cannot read " << profPath << std::endl;
		}

		std::vector<Particle> particles;
		double initialTime;
		int particleSize;
		ifs >> initialTime;
		ifs >> particleSize;
		for (int i = 0; i < particleSize; i++) {
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

		return {initialTime, particles};
	}
};
