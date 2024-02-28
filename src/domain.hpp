#pragma once

#include "common.hpp"

/**
 * @brief represents the domain of the simulation
 */
class Domain {
private:
public:
    double xMin; ///< minimum x coordinate of the domain
    double xMax; ///< maximum x coordinate of the domain
    double yMin; ///< minimum y coordinate of the domain
    double yMax; ///< maximum y coordinate of the domain
    double zMin; ///< minimum z coordinate of the domain
    double zMax; ///< maximum z coordinate of the domain
    double xLength, yLength, zLength;
};
