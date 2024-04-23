#include "csv.hpp"

#include "../particles.hpp"

#include <Eigen/Dense>
#include <csv.h>
#include <fstream>
#include <iostream>
#include <string>

using ParticlesLoader::Csv;
using std::cerr;
using std::endl;

Csv::Csv() {
}

std::pair<double, Particles> Csv::load(const fs::path& path) {
    // load csv data like following
    // 0.0 (start time)
    // 3 (number of particles)
    // type, x, y, z, vx, vy, vz (header)
    // 0,0,0,0,0,0 (type, x, y, z, vx, vy, vz)
    // 1,1,1,1,1,1
    // 2,2,2,2,2,2

    io::CSVReader<7> in(path.string());
    double startTime    = std::stod(in.next_line());
    double numParticles = std::stod(in.next_line());

    in.read_header(io::ignore_extra_column, "type", "x", "y", "z", "vx", "vy", "vz");
    int type;
    double x, y, z, vx, vy, vz;
    Particles particles;
    while (in.read_row(type, x, y, z, vx, vy, vz)) {
        Eigen::Vector3d pos(x, y, z);
        Eigen::Vector3d vel(vx, vy, vz);
        particles.add(Particle(particles.size(), static_cast<ParticleType>(type), pos, vel));
    }

    return std::make_pair(startTime, particles);
}

Csv::~Csv() {
}
