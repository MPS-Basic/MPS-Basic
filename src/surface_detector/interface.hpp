#pragma once

#include "../particle.hpp"
#include "../particles.hpp"

namespace SurfaceDetector {
class Interface {
public:
    /**
     * @brief Whether the particle is on the free surface
     * @param particles particles
     * @param particle particle inside the particles
     * @return Whether the particle is on the free surface
     */
    virtual bool isFreeSurface(const Particles& particles, const Particle& particle) = 0;
    virtual ~Interface()                                                             = default;
};
} // namespace SurfaceDetector
