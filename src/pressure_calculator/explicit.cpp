#include "explicit.hpp"

#include "../refvalues.hpp"

using PressureCalculator::Explicit;

Explicit::Explicit(double re, double soundSpeed, int dimension, double particleDistance) {
    this->soundSpeed = soundSpeed;
    this->n0         = RefValues(dimension, particleDistance, re).n0;
}

Explicit::~Explicit() {
}

std::vector<double> Explicit::calc(Particles& particles) {
    std::vector<double> pressure;
    pressure.resize(particles.size());

#pragma omp parallel for
    for (const auto& pi : particles) {
        if (pi.type == ParticleType::Ghost) {
            pressure[pi.id] = 0;
        } else {
            auto ni  = pi.numberDensity;
            auto c   = this->soundSpeed;
            auto rho = pi.density;

            if (ni > n0) {
                pressure[pi.id] = c * c * rho * (ni - n0) / n0;
            } else {
                pressure[pi.id] = 0;
            }
        }
    }

    return pressure;
}
