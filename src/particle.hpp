#pragma once

#include "Eigen/Dense"
#include "common.hpp"

#include <vector>

/**
 * @brief Enum class for particle type
 */
enum class ParticleType {
    Ghost,     ///< Ghost particle (outside of the domain, not used for calculation)
    Fluid,     ///< Fluid particle
    Wall,      ///< Wall particle
    DummyWall, ///< Dummy wall particle (pressure is not calculated)
};

/**
 * @brief Enum class for fluid state
 */
enum class FluidState {
    Ignored,        ///< Ghost or dummy
    FreeSurface,    ///< free surface particle
    SubFreeSurface, ///< inner particle near free surface
    Inner,          ///< inner fluid particle
    Splash          ///< splash fluid particle
};

/**
 * @brief Class for neighbor particle
 */
class Neighbor {
private:
public:
    int id;          ///< index of the neighbor particle
    double distance; ///< distance between the particle and the neighbor particle

    Neighbor(int id, double distance) {
        this->id       = id;
        this->distance = distance;
    }
};

/**
 * @brief Class for particle in MPS method
 */
class Particle {
private:
public:
    int id;            ///< index of the particle
    ParticleType type; ///< type of the particle

    Eigen::Vector3d position;                               ///< position of the particle
    Eigen::Vector3d velocity;                               ///< velocity of the particle
    Eigen::Vector3d acceleration = Eigen::Vector3d::Zero(); ///< acceleration of the particle
    double pressure              = 0;                       ///< pressure of the particle
    double numberDensity         = 0;                       ///< number density of the particle

    FluidState boundaryCondition = FluidState::Ignored; ///< boundary condition of the particle
    double sourceTerm            = 0;                   ///< source term of the particle
    double minimumPressure       = 0;                   ///< minimum pressure of the particle

    std::vector<Neighbor> neighbors; ///< neighbors of the particle

    /**
     * @brief constructor
     * @param id  index of the particle
     * @param type  type of the particle
     * @param pos  position of the particle
     * @param vel 	velocity of the particle
     */
    Particle(int id, ParticleType type, Eigen::Vector3d pos, Eigen::Vector3d vel);

    /**
     * @brief calculate inverse of density
     * @param density density of the particle
     * @return inverse of density
     */
    double inverseDensity(double& density) const;
};
