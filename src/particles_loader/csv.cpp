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
    io::CSVReader<8> in(path.string());
    double startTime    = std::stod(in.next_line());
    double numParticles = std::stod(in.next_line());

    in.read_header(io::ignore_missing_column, "type", "fluidType", "x", "y", "z", "vx", "vy", "vz");
    int type;
    std::string fluidType = std::string();
    double x, y, z, vx, vy, vz;
    Particles particles;
    while (in.read_row(type, fluidType, x, y, z, vx, vy, vz)) {
        Eigen::Vector3d pos(x, y, z);
        Eigen::Vector3d vel(vx, vy, vz);
        particles.add(Particle(particles.size(), static_cast<ParticleType>(type), pos, vel, fluidType));
    }

    return std::make_pair(startTime, particles);
}

Csv::~Csv() {
}
