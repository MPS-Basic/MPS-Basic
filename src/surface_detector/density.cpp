#include "density.hpp"

using SurfaceDetector::Density;

bool Density::isFreeSurface(const Particles& particles, const Particle& particle) {
    return particle.numberDensity < thresholdRatio * n0;
}

Density::~Density() {
}

Density::Density(double thresholdRatio, double n0) {
    this->thresholdRatio = thresholdRatio;
    this->n0             = n0;
}
