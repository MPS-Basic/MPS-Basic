#include "number_density.hpp"

using SurfaceDetector::NumberDensity;

bool NumberDensity::isFreeSurface(const Particles& particles, const Particle& particle) {
    return particle.numberDensity < thresholdRatio * n0;
}

NumberDensity::~NumberDensity() {
}

NumberDensity::NumberDensity(double thresholdRatio, double n0) {
    this->thresholdRatio = thresholdRatio;
    this->n0             = n0;
}
