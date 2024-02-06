#pragma once
#include "common.hpp"
#include <yaml-cpp/yaml.h>

using namespace Eigen;

class Settings {
private:
public:
	// computational condition
	int dim;
	double particleDistance;
	double dt;
	double finishTime;
	double outputPeriod;
	double cflCondition;
	int numPhysicalCores;

	// domain
	Domain domain;

	// physical properties
	double kinematicViscosity;
	double fluidDensity;

	// gravity
	Vector3d gravity;

	// free surface detection
	double surfaceDetectionRatio;

	// parameters for pressure Poisson equation
	double compressibility;
	double relaxationCoefficientForPressure;

	// collision
	double collisionDistance;
	double coefficientOfRestitution;

	// effective radius
	double radiusForNumberDensity;
	double radiusForGradient;
	double radiusForLaplacian;

	Settings() {
	}

	void load(YAML::Node input) {
		// computational conditions
		dim              = input["dim"].as<int>();
		particleDistance = input["particleDistance"].as<double>();
		dt               = input["dt"].as<double>();
		finishTime       = input["finishTime"].as<double>();
		outputPeriod     = input["outputPeriod"].as<double>();
		cflCondition     = input["cflCondition"].as<double>();
		numPhysicalCores = input["numPhysicalCores"].as<int>();

		// physical properties
		fluidDensity       = input["fluidDensity"].as<double>();
		kinematicViscosity = input["kinematicViscosity"].as<double>();

		// gravity
		gravity[0] = input["gravity"][0].as<double>();
		gravity[1] = input["gravity"][1].as<double>();
		gravity[2] = input["gravity"][2].as<double>();

		// free surface detection
		surfaceDetectionRatio = input["surfaceDetectionRatio"].as<double>();

		// pressure Poisson equation
		compressibility                  = input["compressibility"].as<double>();
		relaxationCoefficientForPressure = input["relaxationCoefficientForPressure"].as<double>();

		// collision
		collisionDistance        = input["collisionDistanceRatio"].as<double>() * particleDistance;
		coefficientOfRestitution = input["coefficientOfRestitution"].as<double>();

		// effective radius
		radiusForNumberDensity = input["radiusRatioForNumberDensity"].as<double>() * particleDistance;
		radiusForGradient      = input["radiusRatioForGradient"].as<double>() * particleDistance;
		radiusForLaplacian     = input["radiusRatioForLaplacian"].as<double>() * particleDistance;

		// domain
		domain.xMin    = input["domainMin"][0].as<double>();
		domain.xMax    = input["domainMax"][0].as<double>();
		domain.yMin    = input["domainMin"][1].as<double>();
		domain.yMax    = input["domainMax"][1].as<double>();
		domain.zMin    = input["domainMin"][2].as<double>();
		domain.zMax    = input["domainMax"][2].as<double>();
		domain.xLength = domain.xMax - domain.xMin;
		domain.yLength = domain.yMax - domain.yMin;
		domain.zLength = domain.zMax - domain.zMin;
	}
};
