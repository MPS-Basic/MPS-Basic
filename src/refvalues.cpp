#include "refvalues.hpp"

#include "weight.hpp"

#include <cassert>
#include <cmath>
#include <utility>

RefValues::RefValues(int dim, double particleDistance, double re) {
    assert(dim == 2 || dim == 3);
    assert(particleDistance < re);
    int iZ_start = -4;
    int iZ_end   = 5;
    if (dim == 2) {
        iZ_start = 0;
        iZ_end   = 1;
    }

    this->n0     = 0.0;
    this->lambda = 0.0;
    for (int iX = -4; iX < 5; iX++) {
        for (int iY = -4; iY < 5; iY++) {
            for (int iZ = iZ_start; iZ < iZ_end; iZ++) {
                if (((iX == 0) && (iY == 0)) && (iZ == 0))
                    continue;

                double xj   = particleDistance * (double) (iX);
                double yj   = particleDistance * (double) (iY);
                double zj   = particleDistance * (double) (iZ);
                double dis2 = xj * xj + yj * yj + zj * zj;
                double dis  = sqrt(dis2);
                n0 += weight(dis, re);
                lambda += dis2 * weight(dis, re);
            }
        }
    }
    this->lambda /= this->n0;
}
