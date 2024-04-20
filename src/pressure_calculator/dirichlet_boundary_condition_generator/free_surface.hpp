#pragma once

#include "../../particle.hpp"
#include "../../surface_detector/interface.hpp"
#include "interface.hpp"

#include <memory>

namespace PressureCalculator::DirichletBoundaryConditionGenerator {
/**
 * @brief Dirichlet Boundary Condition Generator that sets the free surface pressure to 0
 */
class FreeSurface : public Interface {
public:
    /**
     * @brief Generate Dirichlet boundary condition
     * @param particles Particles
     * @return Dirichlet boundary condition
     */
    DirichletBoundaryCondition generate(Particles& particles) override;
    ~FreeSurface() override;
    FreeSurface(std::unique_ptr<SurfaceDetector::Interface>&& surfaceDetector);

private:
    std::unique_ptr<SurfaceDetector::Interface> surfaceDetector;
    void setBoundaryCondition(Particles& particles);
};
}; // namespace PressureCalculator::DirichletBoundaryConditionGenerator
