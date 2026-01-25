#include "vtu.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using ParticlesLoader::Vtu;
using std::cerr;
using std::endl;

Vtu::Vtu() {
}

std::pair<double, Particles> Vtu::load(const fs::path& path, double defaultDensity) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        cerr << "cannot read vtu file: " << fs::absolute(path) << endl;
        std::exit(-1);
    }

    std::string line;
    double startTime = NAN;
    Particles particles;

    std::vector<Eigen::Vector3d> positions;
    std::vector<Eigen::Vector3d> velocities;
    std::vector<int> types;
    std::vector<int> fluidTypes;
    std::vector<double> densities;

    // Parse VTU file
    while (std::getline(ifs, line)) {
        // Extract time from FieldData
        if (line.find("<DataArray type='Float64' Name='Time'") != std::string::npos) {
            std::getline(ifs, line);
            startTime = std::stod(line);
        }

        // Extract particle positions
        if (line.find("<DataArray NumberOfComponents='3' type='Float64' Name='position'") != std::string::npos) {
            while (std::getline(ifs, line) && line.find("</DataArray>") == std::string::npos) {
                std::istringstream iss(line);
                double x, y, z;
                iss >> x >> y >> z;
                positions.emplace_back(x, y, z);
            }
        }

        // Extract velocities
        if (line.find("<DataArray NumberOfComponents='3' type='Float64' Name='Velocity'") != std::string::npos) {
            while (std::getline(ifs, line) && line.find("</DataArray>") == std::string::npos) {
                std::istringstream iss(line);
                double vx, vy, vz;
                iss >> vx >> vy >> vz;
                velocities.emplace_back(vx, vy, vz);
            }
        }

        // Extract particle types
        if (line.find("<DataArray NumberOfComponents='1' type='Int32' Name='Particle Type'") != std::string::npos) {
            while (std::getline(ifs, line) && line.find("</DataArray>") == std::string::npos) {
                int type;
                std::istringstream iss(line);
                iss >> type;
                types.push_back(type);
            }
        }

        // Extract fluid types
        if (line.find("<DataArray NumberOfComponents='1' type='Int32' Name='Fluid Type'") != std::string::npos) {
            while (std::getline(ifs, line) && line.find("</DataArray>") == std::string::npos) {
                int fluidType;
                std::istringstream iss(line);
                iss >> fluidType;
                fluidTypes.push_back(fluidType);
            }
        }

        // Extract densities
        if (line.find("<DataArray NumberOfComponents='1' type='Float64' Name='Density'") != std::string::npos) {
            while (std::getline(ifs, line) && line.find("</DataArray>") == std::string::npos) {
                double density;
                std::istringstream iss(line);
                iss >> density;
                densities.push_back(density);
            }
        }
    }

    // Construct particles
    size_t numParticles = positions.size();
    for (size_t i = 0; i < numParticles; ++i) {
        Eigen::Vector3d pos = positions[i];
        Eigen::Vector3d vel = (i < velocities.size()) ? velocities[i] : Eigen::Vector3d(0, 0, 0);
        int type            = (i < types.size()) ? types[i] : 0;
        int fluidType       = (i < fluidTypes.size()) ? fluidTypes[i] : 0;
        double density      = (i < densities.size() && densities[i] >= 0) ? densities[i] : defaultDensity;

        particles.add(Particle(particles.size(), static_cast<ParticleType>(type), pos, vel, density, fluidType));
    }

    return {startTime, particles};
}

Vtu::~Vtu() {
}
