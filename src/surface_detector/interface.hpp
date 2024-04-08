#pragma once

#include "../particle.hpp"

namespace SurfaceDetector {
class Interface {
public:
    /**
     * @brief
     * @param particles particles
     * @return fluid state of particles
     */
    virtual bool isFreeSurface(const Particle& particle) = 0;
    virtual ~Interface()                                 = default;
};
} // namespace SurfaceDetector
