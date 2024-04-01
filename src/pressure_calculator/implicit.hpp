#pragma once

#include "../particles.hpp"
#include "../refvalues.hpp"
#include "interface.hpp"

#include <Eigen/Sparse>
#include <vector>

namespace PressureCalculator {

class Implicit : public Interface {
public:
    /**
     * @brief calculate pressure
     * @param particles particles
     */
    std::vector<double> calc(const Particles& particles) override;
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

    Particles particles;

    Eigen::SparseMatrix<double, Eigen::RowMajor>
        coefficientMatrix;      ///< Coefficient matrix for pressure Poisson equation
    Eigen::VectorXd sourceTerm; ///< Source term for pressure Poisson equation
    Eigen::VectorXd pressure;   ///< Solution of pressure Poisson equation

    /**
     * @brief set source term of pressure Poisson equation
     * @details
     * The source term of the pressure Poisson equation is calculated as follows:
     * \f[
     * b_i = \frac{\gamma}{\Delta t^2}\frac{n_i-n^0}{n^0}
     * \f]
     */
    void setSourceTerm();

    /**
     * @brief set coefficient matrix of pressure Poisson equation
     * @param re  effective radius \f$r_e\f$
     * @details
     * Applying Laplacian model to pressure Poisson equation, the coefficient matrix is calculated as follows:
     * \f[
     *	-\frac{1}{\rho^0}\langle\nabla^2 P\rangle_i^{k+1} = b_i.
     * \f]
     */
    void setMatrix();

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
