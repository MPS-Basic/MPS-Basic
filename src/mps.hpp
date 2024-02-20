#pragma once

#include "bucket.hpp"
#include "common.hpp"
#include "domain.hpp"
#include "input.hpp"
#include "refvalues.hpp"
#include "settings.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>

/**
 * @brief MPS simulation class
 *
 * @details Executes the MPS simulation. This class does not handle the
 * simulation process itself, but only the calculation of the MPS method.
 */
class MPS {
public:
	Settings settings;               ///< Settings for the simulation
	RefValues refValues;             ///< Reference values for the simulation (\f$n^0\f$, \f$\lambda^0\f$)
	std::vector<Particle> particles; ///< Particles in the simulation
	Bucket bucket;                   ///< Bucket for neighbor search
	Domain domain;                   ///< Domain of the simulation

	// pressure Poisson equation
	Eigen::SparseMatrix<double, Eigen::RowMajor>
	    coefficientMatrix;      ///< Coefficient matrix for pressure Poisson equation
	Eigen::VectorXd sourceTerm; ///< Source term for pressure Poisson equation
	Eigen::VectorXd pressure;   ///< Solution of pressure Poisson equation

	double courant; ///< Maximum courant number among all particles

	MPS() = default;

	MPS(const Input& input);

	void init();

	void stepForward();

private:
	/**
	 * @brief calculate gravity term
	 */
	void calGravity();

	/**
	 * @brief calculate viscosity term of Navier-Stokes equation
	 * @param re effective radius \f$r_e\f$
	 * @details
	 * The viscosity term of the Navier-Stokes equation is calculated as follows:
	 * \f[
	 * \nu\langle \nabla^2\mathbf{u}\rangle_i = \nu\frac{2 d}{n^0\lambda^0}\sum_{j\neq i} (\mathbf{u}_j - \mathbf{u}_i)
	 * w_{ij} \f]
	 */
	void calViscosity(const double& re);

	/**
	 * @brief move particles in prediction step
	 * @details
	 * The position and velocity of each particle are updated as
	 * \f[
	 * \mathbf{u}_i^* = \mathbf{u}_i^k + (\nu\langle \nabla^2\mathbf{u}\rangle^k_i+\mathbf{g})\Delta t.
	 * \f]
	 */
	void moveParticle();

	/**
	 *@brief calculate collision between particles when they are too close
	 */
	void collision();

	/**
	 * @brief calculate pressure using pressure Poisson equation
	 */
	void calPressure() {
		calNumberDensity(settings.re_forNumberDensity);
		setBoundaryCondition();
		setSourceTerm();
		setMatrix(settings.re_forLaplacian);
		solveSimultaneousEquations();
		removeNegativePressure();
		setMinimumPressure(settings.re_forGradient);
	}

	/**
	 * @brief calculate number density of each particle
	 * @param re effective radius \f$r_e\f$
	 */
	void calNumberDensity(const double& re);

	/**
	 *@brief set boundary condition of pressure Poisson equation
	 */
	void setBoundaryCondition();

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
	void setMatrix(const double& re);

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

	/**
	 * @brief set minimum pressure for pressure gradient calculation
	 * @param re effective radius \f$r_e\f$
	 */
	void setMinimumPressure(const double& re);
	/**
	 * @brief calculate pressure gradient term
	 * @param re
	 * @details
	 * The pressure gradient term of the Navier-Stokes equation is calculated as
	 * \f[
	 * -\frac{1}{\rho^0}\langle\nabla P\rangle_i = -\frac{1}{\rho^0}\frac{d}{n^0}\sum_{j\neq i}
	 * \frac{P_j-P'_i}{\|\mathbf{r}_{ij}\|^2}\mathbf{r}_{ij} w_{ij} \f] where \f$P'_i\f$ is the minimum pressure of
	 * the particle \f$i\f$.
	 */
	void calPressureGradient(const double& re);

	/**
	 * @brief move particles in correction step
	 * @details
	 * The position and velocity of each particle are updated as
	 * \f[
	 * \mathbf{u}_i^{k+1} = \mathbf{u}_i^* - \frac{1}{\rho^0} \langle\nabla P^{k+1} \rangle_i \Delta t.
	 * \f]
	 */
	void moveParticleUsingPressureGradient();

	/**
	 * @brief calculate Courant number
	 */
	void calCourant();

	/**
	 * @brief search neighbors of each particle
	 * @param re effective radius
	 */
	void setNeighbors(const double& re);
};
