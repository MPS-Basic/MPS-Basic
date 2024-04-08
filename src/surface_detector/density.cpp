#include "density.hpp"

using SurfaceDetector::Density;

bool Density::isFreeSurface(const Particle& particle) {
    return particle.numberDensity < beta * n0;
}

Density::~Density() {
}

Density::Density(double beta, double n0) {
    this->beta = beta;
    this->n0   = n0;
}
