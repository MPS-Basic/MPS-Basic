#pragma once

#include "common.hpp"
#include "domain.hpp"
#include <Eigen/Dense>
#include <filesystem>

/// @brief Struct for settings of calculation
struct Settings {
	// computational condition
	int dim;                 ///< Dimension of the simulation
	double particleDistance; ///< Initial distance between particles
	double dt;               ///< Time step
	double finishTime;       ///< Finish time of the simulation
	double outputPeriod;     ///< Output period of the simulation
	double cflCondition;     ///< CFL condition
	int numPhysicalCores;    ///< Number of cores to calculate

	Domain domain; ///< domain of the simulation

	// physical properties
	double kinematicViscosity; ///< Kinematic viscosity
	double fluidDensity;       ///< Fluid density

	// gravity
	Eigen::Vector3d gravity; ///< Gravity

	// free surface detection
	double surfaceDetectionRatio; ///< Ratio for free surface detection

	// parameters for pressure Poisson equation
	double compressibility;                  ///< Compressibility of the fluid
	double relaxationCoefficientForPressure; ///< Relaxation coefficient for pressure

	// collision
	double collisionDistance;        ///< Distance for collision detection
	double coefficientOfRestitution; ///< Coefficient of restitution

	// effective radius
	double re_forNumberDensity; ///< Effective radius for number density
	double re_forGradient;      ///< Effective radius for gradient
	double re_forLaplacian;     ///< Effective radius for Laplacian
	double reMax;               ///< Maximum of effective radius

	// profpath
	std::filesystem::path profPath; ///< Path for input particle file
};
