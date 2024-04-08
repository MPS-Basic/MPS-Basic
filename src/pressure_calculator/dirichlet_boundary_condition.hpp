#pragma once

#include <unordered_map>

namespace PressureCalculator {

/**
 * @brief Dirichlet boundary condition
 *
 * @details Dirichlet boundary condition is used to set the pressure of particles on the boundary of the fluid domain.
 * This class controls the id of particles that attach Dirichlet boundary conditions and the value of pressure for those
 * particles.
 */
class DirichletBoundaryCondition {
public:
    /**
     * @brief Check if the boundary condition contains the particle
     * @param id Particle id
     * @return True if the boundary condition contains the particle
     */
    bool contains(int id) const;

    /**
     * @brief Get the boundary condition value
     * @param id Particle id
     * @return Boundary condition value
     */
    double value(int id) const;

    /**
     * @brief Set the boundary condition value
     * @param id Particle id
     * @param value Boundary condition value
     */
    void set(int id, double value);

    DirichletBoundaryCondition() = default;

private:
    /**
     * @brief boundary condition values
     *
     * key: particle id
     * value: boundary condition value
     */
    std::unordered_map<int, double> values;
};

} // namespace PressureCalculator
