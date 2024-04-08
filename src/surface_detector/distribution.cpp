#include "distribution.hpp"

#include <Eigen/Dense>

using SurfaceDetector::Distribution;

bool Distribution::isFreeSurface(const Particles& particles, const Particle& particle) {
    Eigen::Vector3d rij_sum = Eigen::Vector3d::Zero();
    for (auto& neighbor : particle.neighbors) {
        auto& pj = particles[neighbor.id];

        rij_sum += pj.position - particle.position;
    }

    if (abs(rij_sum.x()) > threshold * particleDistance) {
        return true;

    } else if (abs(rij_sum.y()) > threshold * particleDistance) {
        return true;

    } else if (abs(rij_sum.z()) > threshold * particleDistance) {
        return true;

    } else {
        return false;
    }
}

Distribution::Distribution(double particleDistance, double threshold)
    : particleDistance(particleDistance), threshold(threshold) {
}

Distribution::~Distribution() {
}
