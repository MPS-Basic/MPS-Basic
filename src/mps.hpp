#pragma once

#include "bucket.hpp"
#include "common.hpp"
#include "domain.hpp"
#include "input.hpp"
#include "neighbor_searcher.hpp"
#include "particles.hpp"
#include "pressure_calculator/interface.hpp"
#include "refvalues.hpp"
#include "settings.hpp"
#include "surface_detector/interface.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <memory>
#include <vector>

/**
 * @brief MPS simulation class
 *
 * @details Executes the MPS simulation. This class does not handle the
 * simulation process itself, but only the calculation of the MPS method.
 */
class MPS {
public:
    Settings settings;                   ///< Settings for the simulation
    RefValues refValuesForNumberDensity; ///< Reference values for the simulation (\f$n^0\f$, \f$\lambda^0\f$)
    RefValues refValuesForLaplacian;     ///< Reference values for the simulation (\f$n^0\f$, \f$\lambda^0\f$)
    RefValues refValuesForGradient;      ///< Reference values for the simulation (\f$n^0\f$, \f$\lambda^0\f$)
    Particles particles;                 ///< Particles in the simulation
    Domain domain{};                     ///< Domain of the simulation

    std::unique_ptr<PressureCalculator::Interface> pressureCalculator; ///< Interface for pressure calculation

    double courant{}; ///< Maximum courant number among all particles

    MPS() = default;

    MPS(const Input& input,
        std::unique_ptr<PressureCalculator::Interface>&& pressureCalculator,
        std::unique_ptr<SurfaceDetector::Interface>&& surfaceDetector);

    void stepForward();

private:
    NeighborSearcher neighborSearcher;                           ///< Neighbor searcher for neighbor search
    std::unique_ptr<SurfaceDetector::Interface> surfaceDetector; ///< Interface for free surface detection

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
     * @brief calculate number density of each particle
     * @param re effective radius \f$r_e\f$
     */
    void calNumberDensity(const double& re);

    /**
     *@brief set boundary condition of pressure Poisson equation
     */
    void setBoundaryCondition();

    bool isFreeSurface(const Particle& pi);

    /**
     * @brief check if particle distribution is biased.
     * @details
     * Particle number density can be low even in the fluid region, making the free surface detection based on particle
     * number density vulnerable. Even when a specific particle's the particle number density is lower than criteria,
     * it can be considered as inner particle if the particle distribution around the particle is not biased. In this
     * way we can avoid the free surface detection error.
     * This is based on Khayyer et al. (https://doi.org/10.1016/j.apor.2009.06.003)
     *
     * @param pi Particle to check
     * @return true particle distribution is biased = free surface
     * @return false particle distribution is not biased = not free surface
     */
    bool isParticleDistributionBiased(const Particle& pi);

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

    void addSpacePotentialParticles();

    void addSpacePotentialParticle(Particle& particle);

    void removeSpacePotentialParticles();
};
