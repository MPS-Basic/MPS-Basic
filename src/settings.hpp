#pragma once

#include "common.hpp"
#include "domain.hpp"
#include <filesystem>
#include <Eigen/Dense>

struct Settings {
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
	Eigen::Vector3d gravity;

	// free surface detection
	double surfaceDetectionRatio;

	// parameters for pressure Poisson equation
	double compressibility;
	double relaxationCoefficientForPressure;

	// collision
	double collisionDistance;
	double coefficientOfRestitution;

	// effective radius
	double re_forNumberDensity;
	double re_forGradient;
	double re_forLaplacian;
	double reMax;

	// profpath
	std::filesystem::path profPath;
};
