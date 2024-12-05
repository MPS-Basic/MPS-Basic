#pragma once

#include "common.hpp"
#include "domain.hpp"

#include <Eigen/Dense>
#include <filesystem>

/**
 * @brief Struct for settings of calculation
 *
 * @details This struct contains the settings for the simulation. It is used to
 * load the settings from the input file.
 */
struct Settings {
    // computational condition
    int dim{};                 ///< Dimension of the simulation
    double particleDistance{}; ///< Initial distance between particles
    double dt{};               ///< Time step
    double endTime{};          ///< End time of the simulation
    double outputPeriod{};     ///< Output period of the simulation
    double cflCondition{};     ///< CFL condition
    int numPhysicalCores{};    ///< Number of cores to calculate

    Domain domain{}; ///< domain of the simulation

    // physical properties
    double kinematicViscosity{}; ///< Kinematic viscosity
    double defaultDensity{};     ///< default density for fluid and wall particles.

    // gravity
    Eigen::Vector3d gravity; ///< Gravity

    // free surface detection
    // surface detection based on number density
    double surfaceDetection_numberDensity_threshold{}; ///< threshold ratio of number density for free surface detection
    // surface detection based on particle distribution
    bool surfaceDetection_particleDistribution{}; ///< flag for free surface detection based on particle distribution
    double surfaceDetection_particleDistribution_threshold{};

    // pressure calculation method
    std::string pressureCalculationMethod{}; ///< Method for pressure calculation
    // for Implicit
    double compressibility{};                  ///< Compressibility of the fluid for Implicit method
    double relaxationCoefficientForPressure{}; ///< Relaxation coefficient for pressure for Implicit method
    // for Explicit
    double soundSpeed{}; ///< Speed of sound for Explicit method

    // collision
    double collisionDistance{};        ///< Distance for collision detection
    double coefficientOfRestitution{}; ///< Coefficient of restitution

    // effective radius
    double re_forNumberDensity{}; ///< Effective radius for number density
    double re_forGradient{};      ///< Effective radius for gradient
    double re_forLaplacian{};     ///< Effective radius for Laplacian
    double reMax{};               ///< Maximum of effective radius

    // i/o
    std::filesystem::path particlesPath; ///< Path for input particle file
    bool outputVtkInBinary{};            ///< Flag for saving VTK file in binary format
};
