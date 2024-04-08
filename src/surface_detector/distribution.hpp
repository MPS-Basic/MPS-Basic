#pragma once

#include "interface.hpp"

namespace SurfaceDetector {
class Distribution : public Interface {
public:
    bool isFreeSurface(const Particles& particles, const Particle& particle) override;
    ~Distribution() override;

    Distribution(double n0, double particleDistance, double distributionThreshold, double numberDensityThreshold);

private:
    double n0;
    double particleDistance;
    double distributionThreshold;
    double numberDensityThreshold;

    bool mainDetection(const Particles& particles, const Particle& particle);
    bool subDetection(const Particles& particles, const Particle& particle);
};
} // namespace SurfaceDetector
