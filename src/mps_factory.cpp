#include "mps_factory.hpp"

#include "pressure_calculator/dirichlet_boundary_condition_generator/free_surface.hpp"
#include "pressure_calculator/explicit.hpp"
#include "pressure_calculator/implicit.hpp"
#include "surface_detector/distribution.hpp"
#include "surface_detector/number_density.hpp"

namespace DirichletBoundaryConditionGenerator = PressureCalculator::DirichletBoundaryConditionGenerator;

MPS MPSFactory::create(const Input& input) {
    Eigen::Vector3d gravity = Eigen::Vector3d::Zero();
    if (input.settings.xyzInput) {
        gravity = input.settings.gXyzInput;
    } else {
        double gNorm  = input.settings.gNorm;
        double gAngle = input.settings.gAngle;
        double theta  = gAngle * M_PI / 180.0; // Convert angle to radians
        gravity << gNorm * sin(theta), -gNorm * cos(theta), 0.0; // Gravity in XY plane
    }

    RefValues refValuesForNumberDensity(
        input.settings.dim,
        input.settings.particleDistance,
        input.settings.re_forNumberDensity
    );

    std::unique_ptr<SurfaceDetector::Interface> surfaceDetector;
    if (input.settings.surfaceDetection_particleDistribution) {
        surfaceDetector.reset(new SurfaceDetector::Distribution(
            refValuesForNumberDensity.n0,
            input.settings.particleDistance,
            input.settings.surfaceDetection_particleDistribution_threshold,
            input.settings.surfaceDetection_numberDensity_threshold
        ));
    } else {
        surfaceDetector.reset(new SurfaceDetector::NumberDensity(
            input.settings.surfaceDetection_numberDensity_threshold,
            refValuesForNumberDensity.n0
        ));
    }

    std::unique_ptr<DirichletBoundaryConditionGenerator::Interface> DirichletBoundaryConditionGenerator;
    DirichletBoundaryConditionGenerator.reset(
        new DirichletBoundaryConditionGenerator::FreeSurface(std::move(surfaceDetector))
    );

    std::unique_ptr<PressureCalculator::Interface> pressureCalculator;
    if (input.settings.pressureCalculationMethod == "Implicit") {
        pressureCalculator.reset(new PressureCalculator::Implicit(
            input.settings.dim,
            input.settings.particleDistance,
            input.settings.re_forNumberDensity,
            input.settings.re_forLaplacian,
            input.settings.dt,
            input.settings.compressibility,
            input.settings.relaxationCoefficientForPressure,
            std::move(DirichletBoundaryConditionGenerator)
        ));
    } else if (input.settings.pressureCalculationMethod == "Explicit") {
        pressureCalculator.reset(new PressureCalculator::Explicit(
            input.settings.re_forNumberDensity,
            input.settings.soundSpeed,
            input.settings.dim,
            input.settings.particleDistance
        ));
    } else {
        std::cerr << "Invalid pressure calculation method: " << input.settings.pressureCalculationMethod << std::endl;
        std::cerr << "Please select either Implicit or Explicit." << std::endl;
        std::exit(-1);
    }

    return MPS(input, gravity, std::move(pressureCalculator), std::move(surfaceDetector));
}
