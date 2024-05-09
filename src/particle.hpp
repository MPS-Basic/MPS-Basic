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
    int fluidType = 0; ///< type of the fluid. This is used for simulation of multiple fluid types. When treating only
                       ///< one fluid, this property is not used. Default value is 0.

    Eigen::Vector3d position;                               ///< position of the particle
    Eigen::Vector3d velocity;                               ///< velocity of the particle
    Eigen::Vector3d acceleration = Eigen::Vector3d::Zero(); ///< acceleration of the particle
    double pressure              = 0;                       ///< pressure of the particle
    double numberDensity         = 0;                       ///< number density of the particle
    double density               = 0; ///< density of the particle. It is used only for fluid and wall particles.

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
     * @param density density of the particle
     * @param fluidType type of the fluid (optional). Default value is 0.
     */
    Particle(int id, ParticleType type, Eigen::Vector3d pos, Eigen::Vector3d vel, double density, int fluidType = 0);

    /**
     * @brief calculate inverse of density for collision
     * @return inverse of density
     * @warning This function returns 0 for wall particles. This is because we don't want to move wall particles in the
     * collision process. You should not use this function outside of the collision process. If you want to, you should
     * change this function name correctly.
     */
    double inverseDensityForCollision() const;
};
