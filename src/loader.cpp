#include "loader.hpp"
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

Input Loader::load(const fs::path& settingPath) {
	Input input;
	input.settings = loadSettingYaml(settingPath);

	auto [startTime, particles] = loadParticleProf(input.settings.profPath);
	input.startTime             = startTime;
	input.particles             = particles;

	std::cout << "Output directory: " << input.settings.outputDirectory << std::endl;
	std::filesystem::create_directories(input.settings.outputDirectory);

	auto outputSettingPath = input.settings.outputDirectory / settingPath.filename();
	if (std::filesystem::exists(outputSettingPath)) {
		std::cerr << "setting file already exists in the output directory: " << outputSettingPath << std::endl;
		std::exit(-1);
	} else {
		std::filesystem::copy_file(settingPath, outputSettingPath);
	}
	auto outputProfPath = input.settings.outputDirectory / input.settings.profPath.filename();
	if (std::filesystem::exists(outputProfPath)) {
		std::cerr << "prof file already exists in the output directory: " << outputProfPath << std::endl;
		std::exit(-1);
	} else {
		std::filesystem::copy_file(input.settings.profPath, outputProfPath);
	}
	return input;
}

Settings Loader::loadSettingYaml(const fs::path& settingPath) {
	YAML::Node yaml = YAML::LoadFile(settingPath.string());

	Settings s;

	// computational conditions
	s.dim              = yaml["dim"].as<int>();
	s.particleDistance = yaml["particleDistance"].as<double>();
	s.dt               = yaml["dt"].as<double>();
	s.endTime          = yaml["endTime"].as<double>();
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

	auto yamlDir = settingPath.parent_path();
	// outputDirectory path
	auto relativeOutputDirectory = yaml["outputDirectory"].as<std::string>();
	s.outputDirectory            = std::filesystem::weakly_canonical(yamlDir / relativeOutputDirectory);
	// profpath
	auto relativeProfPath = yaml["profPath"].as<std::string>();
	s.profPath            = std::filesystem::weakly_canonical(yamlDir / relativeProfPath);

	return s;
}

std::pair<double, std::vector<Particle>> Loader::loadParticleProf(const fs::path& profPath) {
	std::ifstream ifs;
	ifs.open(profPath);
	if (ifs.fail()) {
		std::cerr << "cannot read prof file: " << std::filesystem::absolute(profPath) << std::endl;
		std::exit(-1);
	}

	std::vector<Particle> particles;
	double startTime;
	int particleSize;
	ifs >> startTime;
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

	return {startTime, particles};
}