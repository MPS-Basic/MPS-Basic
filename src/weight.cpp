#include "weight.hpp"

#include <cassert>
#include <iostream>

Weight::Weight(WeightType weightType) {
    this->weightType = weightType;
}

double Weight::weightFn(double dis, double re) {
    assert(dis > 0.0);

    if (dis >= re) {
        return 0.0;
    } else {
        if (weightType == WeightType::SemiImplicit) {
            return (re / dis) - 1.0;
        } else if (weightType == WeightType::Explicit) {
            return (re / dis) + (dis / re) - 2.0;
        } else if (weightType == WeightType::ExplicitGrad) {
            return (re / dis) - (dis / re);
        }
        std::cerr << "ERROR: Unknown weight type." << std::endl;
        std::exit(-1);
    }
}
