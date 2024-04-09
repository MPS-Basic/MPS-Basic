#pragma once

#include "interface.hpp"

namespace SurfaceDetector {
/**
 * @brief Detects free surface based on number density and assists evaluation by particle distribution
 * @details Proposed by Khayyer et al. (2009). A supplementary judgment based on particle distribution is added to the
 * judgment based on number density. Even if a particle is determined to be a surface particle by number density, it is
 * not considered a surface particle if there is no bias in the arrangement of the particles.
 */
class Distribution : public Interface {
public:
    bool isFreeSurface(const Particles& particles, const Particle& particle) override;
    ~Distribution() override;

    Distribution(
        double n0, double particleDistance, double distributionThresholdRatio, double numberDensityThresholdRatio
    );

private:
    double n0;                          ///< Reference value for number density
    double particleDistance;            ///< Particle distance
    double distributionThresholdRatio;  ///< Threshold ratio for particle distribution
    double numberDensityThresholdRatio; ///< Threshold ratio for number density

    bool mainDetection(const Particles& particles, const Particle& particle);
    bool subDetection(const Particles& particles, const Particle& particle);
};
} // namespace SurfaceDetector
