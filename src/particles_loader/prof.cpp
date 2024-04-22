#include "prof.hpp"

using ParticlesLoader::Prof;

std::pair<double, Particles> Prof::load(const fs::path& path) {
    std::ifstream ifs;
    ifs.open(profPath);
    if (ifs.fail()) {
        cerr << "cannot read prof file: " << fs::absolute(profPath) << endl;
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
        if (type != ParticleType::Ghost) {
            particles.add(Particle(particles.size(), type, pos, vel));
        }
    }
    ifs.close();
    ifs.clear();

    return {startTime, particles};
}

Prof::~Prof() {
}
