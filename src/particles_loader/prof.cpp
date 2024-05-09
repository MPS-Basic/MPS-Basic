#include "prof.hpp"

#include <fstream>
#include <iostream>

using ParticlesLoader::Prof;
using std::cerr;
using std::endl;

Prof::Prof() {
}

std::pair<double, Particles> Prof::load(const fs::path& path, double defaultDensity) {
    std::ifstream ifs;
    ifs.open(path);
    if (ifs.fail()) {
        cerr << "cannot read prof file: " << fs::absolute(path) << endl;
        std::exit(-1);
    }

    Particles particles;
    double startTime = NAN;
    int particleSize = 0;
    ifs >> startTime;
    ifs >> particleSize;
    for (int i = 0; i < particleSize; i++) {
        int type_int = 0;
        ParticleType type;
        Eigen::Vector3d pos, vel;

        ifs >> type_int;
        ifs >> pos.x() >> pos.y() >> pos.z();
        ifs >> vel.x() >> vel.y() >> vel.z();

        type = static_cast<ParticleType>(type_int);
        particles.add(Particle(particles.size(), type, pos, vel, defaultDensity));
    }

    return {startTime, particles};
}

Prof::~Prof() {
}
