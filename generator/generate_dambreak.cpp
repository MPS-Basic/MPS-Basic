#include "../src/particle.hpp"
#include "../src/particles.hpp"
#include "../src/particles_exporter.hpp"
#include "generator_src/generator_dialogue.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

bool isInside(
    Eigen::Vector3d& position,
    std::vector<double>& x_range,
    std::vector<double>& y_range,
    std::vector<double>& z_range,
    double& eps
);

int main(int argc, char** argv) {
    Particles particles;

    double l0      = 0.025;
    double eps     = 0.01 * l0;
    double density = 1000.0;

    std::vector<double> fluid_x_range{0.0, 1.0};
    std::vector<double> fluid_y_range{0.0, 0.6};

    int ix_begin = round(fluid_x_range.at(0) / l0) - 4;
    int ix_end   = round(fluid_x_range.at(1) / l0) + 4;
    int iy_begin = round(fluid_y_range.at(0) / l0) - 4;
    int iy_end   = round(fluid_y_range.at(1) / l0) + 4;
    std::vector<double> x_range(2), y_range(2), z_range(2);
    z_range = {0.0, 0.0};
    for (int ix = ix_begin; ix <= ix_end; ix++) {
        for (int iy = iy_begin; iy <= iy_end; iy++) {
            Eigen::Vector3d pos((double) (ix) *l0, (double) iy * l0, 0.0);
            ParticleType type = ParticleType::Ghost;

            // dummy wall region
            x_range = {-4.0 * l0, 1.0 + 4.0 * l0};
            y_range = {-4.0 * l0, 0.6};
            if (isInside(pos, x_range, y_range, z_range, eps))
                type = ParticleType::DummyWall;

            // wall region
            x_range = {-2.0 * l0, 1.0 + 2.0 * l0};
            y_range = {-2.0 * l0, 0.6};
            if (isInside(pos, x_range, y_range, z_range, eps))
                type = ParticleType::Wall;

            // wall region
            x_range = {-4.0 * l0, 1.0 + 4.0 * l0};
            y_range = {0.6 - 2.0 * l0, 0.6};
            if (isInside(pos, x_range, y_range, z_range, eps))
                type = ParticleType::Wall;

            // empty region
            x_range = {0.0, 1.0};
            y_range = {0.0, 0.6};
            if (isInside(pos, x_range, y_range, z_range, eps))
                type = ParticleType::Ghost;

            // fluid region
            x_range = {0.0, 0.25};
            y_range = {0.0, 0.50};
            if (isInside(pos, x_range, y_range, z_range, eps))
                type = ParticleType::Fluid;

            if (type != ParticleType::Ghost) {
                Eigen::Vector3d vel = Eigen::Vector3d::Zero();
                particles.add(Particle(particles.size(), type, pos, vel, density));
            }
        }
    }

    std::string parentPath = "input/dambreak";
    GeneratorDialogue gd;
    gd.generatorDialogue(fs::path(parentPath), particles, {".prof", ".vtu"});
}

bool isInside(
    Eigen::Vector3d& pos,
    std::vector<double>& x_range,
    std::vector<double>& y_range,
    std::vector<double>& z_range,
    double& eps
) {
    std::sort(x_range.begin(), x_range.end());
    std::sort(y_range.begin(), y_range.end());
    std::sort(z_range.begin(), z_range.end());

    if (((x_range.at(0) - eps < pos.x()) && (pos.x() < x_range.at(1) + eps)) &&
        ((y_range.at(0) - eps < pos.y()) && (pos.y() < y_range.at(1) + eps)) &&
        ((z_range.at(0) - eps < pos.z()) && (pos.z() < z_range.at(1) + eps)))
        return true;
    else
        return false;
}
