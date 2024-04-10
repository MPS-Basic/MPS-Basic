#pragma once

#include "interface.hpp"

namespace SurfaceDetector {
/**
 * @brief Detects free surface based on the number density
 * @details Surface detection method for standard MPS method. When the number density of the particle is lower than the
 * thresholdRatio * n0, the particle is considered to be on the free surface. A thresholdRatio value of 0.95 to 0.97 is
 * recommended as a rule of thumb (Koshizuka and Oka, 1996).
 */
class NumberDensity : public Interface {
public:
    bool isFreeSurface(const Particles& particles, const Particle& particle) override;
    ~NumberDensity() override;
    NumberDensity(double thresholdRatio, double n0);

private:
    double thresholdRatio; ///< threshold ratio for number density
    double n0;             ///< reference value for number density
};
} // namespace SurfaceDetector
