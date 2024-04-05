#include "dirichlet_boundary_condition.hpp"

using PressureCalculator::DirichletBoundaryCondition;

bool DirichletBoundaryCondition::contains(int id) const {
    return this->values.find(id) != this->values.end();
}

double DirichletBoundaryCondition::value(int id) const {
    return this->values.at(id);
}

void DirichletBoundaryCondition::set(int id, double value) {
    this->values[id] = value;
}
