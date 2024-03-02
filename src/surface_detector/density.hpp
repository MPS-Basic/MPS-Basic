#pragma onece 

#include "interface.hpp"

namespace SurfaceDetector
{
    class Density : public Interface
    {
    public:
        /**
         * @brief detect free surface for all particles
         * @param particles particles
         * @return fluid state of particles
         */
        std::vector<FluidState> isFreeSurface(const std::vector<Particle> &particles) override;
        /**
         * @brief destructor
         * 
         */
        ~Density() override;
    };
} // namespace SurfaceDetector