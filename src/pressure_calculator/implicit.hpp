#pragma once

#include "../particle.hpp"
#include "../refvalues.hpp"
#include "interface.hpp"
#include "pressure_poisson_equation.hpp"

#include <Eigen/Sparse>
#include <vector>

namespace PressureCalculator {

class Implicit : public Interface {
public:
    /**
     * @brief calculate pressure
     * @param particles particles
     */
    std::vector<double> calc(const std::vector<Particle>& particles) override;
    ~Implicit() override;

    Implicit(
        int dimension,
        double particleDistance,
        double re_forGradient,
        double re_forLaplacian,
        double dt,
        double fluidDensity,
        double compressibility,
        double relaxationCoefficient
    );

private:
    int dimension;
    RefValues refValuesForNumberDensity;
    RefValues refValuesForLaplacian;
    double reForNumberDensity;
    double reForLaplacian;
    double dt;
    double fluidDensity;
    double compressibility;
    double relaxationCoefficient;

    std::vector<Particle> particles;

    Eigen::SparseMatrix<double, Eigen::RowMajor>
        coefficientMatrix;        ///< Coefficient matrix for pressure Poisson equation
    Eigen::VectorXd sourceTerm;   ///< Source term for pressure Poisson equation
    std::vector<double> pressure; ///< Solution of pressure Poisson equation

    PressurePoissonEquation ppe;

    void applyBoundaryCondition(const std::vector<Particle>& particles);

    /**
     * @brief If there is no Dirichlet boundary condition on the fluid,
           increase the diagonal terms of the matrix for an exception. This
           allows us to solve the matrix without Dirichlet boundary conditions.
     *
     */
    void exceptionalProcessingForBoundaryCondition();

    /**
     * @brief Solve the pressure Poisson equation
     * @details
     * The pressure Poisson equation is solved using the BiCGSTAB method.
     */
    void solveSimultaneousEquations();

    /**
     * @brief remove negative pressure for stability
     */
    void removeNegativePressure();
};

} // namespace PressureCalculator
