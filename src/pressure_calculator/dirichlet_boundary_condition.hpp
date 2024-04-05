#pragma once

#include <unordered_map>

namespace PressureCalculator {

class DirichletBoundaryCondition {
public:
    bool contains(int id) const;
    double value(int id) const;
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
