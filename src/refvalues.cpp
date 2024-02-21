#include "refvalues.hpp"
#include "weight.hpp"
#include <cmath>

void RefValues::calc(
    int dim, double particleDistance, double re_forNumberDensity, double re_forGradient, double re_forLaplacian) {

	int iZ_start = 0, iZ_end = 0;
	if (dim == 2) {
		iZ_start = 0;
		iZ_end   = 1;
	} else {
		iZ_start = -4;
		iZ_end   = 5;
	}

	this->n0_forNumberDensity = 0.0;
	this->n0_forGradient      = 0.0;
	this->n0_forLaplacian     = 0.0;
	this->lambda              = 0.0;
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
				this->n0_forNumberDensity += weight(dis, re_forNumberDensity);
				this->n0_forGradient += weight(dis, re_forGradient);
				this->n0_forLaplacian += weight(dis, re_forLaplacian);
				this->lambda += dis2 * weight(dis, re_forLaplacian);
			}
		}
	}
	this->lambda /= this->n0_forLaplacian;
}
