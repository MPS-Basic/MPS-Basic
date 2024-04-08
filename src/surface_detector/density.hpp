#pragma once

#include "interface.hpp"

namespace SurfaceDetector {
class Density : public Interface {
public:
    bool isFreeSurface(const Particles& particles, const Particle& particle) override;
    ~Density() override;
    Density(double beta, double n0);

private:
    double beta; ///< threshold ratio for number density
    double n0;   ///< reference value for number density
};
} // namespace SurfaceDetector
