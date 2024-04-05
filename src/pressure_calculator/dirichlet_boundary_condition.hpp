#pragma once

#include <unordered_map>

namespace PressureCalculator {

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
