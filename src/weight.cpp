#include "weight.hpp"

#include <cassert>

double weight(double dis, double re) {
    assert(dis > 0.0);

    if (dis >= re) {
        return 0.0;

    } else {
        return (re / dis) - 1.0;
    }
}
