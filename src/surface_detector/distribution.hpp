#pragma once

#include "interface.hpp"

namespace SurfaceDetector {
class Distribution : public Interface {
public:
    bool isFreeSurface(const Particles& particles, const Particle& particle) override;
    ~Distribution() override;

    Distribution(double particleDistance, double threshold);

private:
    double particleDistance;
    double threshold;
};
} // namespace SurfaceDetector
