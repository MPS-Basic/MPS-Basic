#include "distribution.hpp"

#include <Eigen/Dense>

using SurfaceDetector::Distribution;

bool Distribution::isFreeSurface(const Particles& particles, const Particle& particle) {
    // main: surface, sub: surface -> surface (true)
    // main: surface, sub: inner   -> inner   (false)
    // main: inner,   sub: surface -> inner   (false)
    // main: inner,   sub: inner   -> inner   (false)
    if (mainDetection(particles, particle)) {
        return subDetection(particles, particle);
    } else {
        return false;
    }
}

bool Distribution::mainDetection(const Particles& particles, const Particle& particle) {
    return particle.numberDensity < numberDensityThreshold * n0;
}

bool Distribution::subDetection(const Particles& particles, const Particle& particle) {
    if (particle.neighbors.empty()) {
        // If the particle has no neighbors, it is considered to be a free surface.
        return true;
    }

    Eigen::Vector3d rij_sum = Eigen::Vector3d::Zero();
    for (auto& neighbor : particle.neighbors) {
        auto& pj = particles[neighbor.id];

        rij_sum += pj.position - particle.position;
    }

    auto threshold = distributionThreshold * particleDistance;

    if (abs(rij_sum.x()) > threshold) {
        return true;

    } else if (abs(rij_sum.y()) > threshold) {
        return true;

    } else if (abs(rij_sum.z()) > threshold) {
        return true;

    } else {
        return false;
    }
}

Distribution::Distribution(
    double n0, double particleDistance, double distributionThreshold, double numberDensityThreshold
)
    : n0(n0), particleDistance(particleDistance), distributionThreshold(distributionThreshold),
      numberDensityThreshold(numberDensityThreshold) {
}

Distribution::~Distribution() {
}
