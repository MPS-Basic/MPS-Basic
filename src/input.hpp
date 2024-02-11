#pragma once

#include "settings.hpp"
#include "particle.hpp"
#include <vector>

struct Input {
    Settings settings;
    std::vector<Particle> particles;
    double initialTime;
};
