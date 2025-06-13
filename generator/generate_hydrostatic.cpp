#include "../src/particle.hpp"
#include "../src/particles.hpp"
#include "../src/particles_exporter.hpp"
#include "generator_src/generator_dialogue.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace fs = std::filesystem;

class Box2d {
private:
    Eigen::Vector2d center;
    Eigen::Vector2d box_size;

public:
    Box2d() = default;
    Box2d(double xMin, double xMax, double yMin, double yMax) {
        center << (xMin + xMax) / 2.0, (yMin + yMax) / 2.0;
        box_size << xMax - xMin, yMax - yMin;
    }
    double sdf(const Eigen::Vector2d& r) const {
        auto q = (r - center).array().abs() - box_size.array() / 2.0;
        return q.max(0.0).matrix().norm() + std::min(std::max(q.x(), q.y()), 0.0);
    }
};

int main(int argc, char** argv) {
    Particles particles;

    double l0      = 0.012;
    double density = 1000.0;

    Box2d fluidDomain(-0.18, 0.18, 0, 0.48);
    Box2d emptyDomain(-0.18, 0.18, 0.48, 0.6);
    Box2d wallBB(-0.18 - 2 * l0, 0.18 + 2 * l0, -2 * l0, 0.6);
    Box2d dummyBB(-0.18 - 4 * l0, 0.18 + 4 * l0, -4 * l0, 0.6);

    int id = 0;
    for (int ix = (int) round(-0.18 / l0) - 4; ix < (int) round(0.18 / l0) + 4; ix++) {
        for (int iy = (int) round(0 / l0) - 4; iy < (int) round(0.6 / l0); iy++) {
            Eigen::Vector2d r(l0 * (ix + 0.5), l0 * (iy + 0.5));
            Eigen::Vector3d r_3d(r.x(), r.y(), 0);

            // empty region
            if (emptyDomain.sdf(r) < 0) {
                continue;
            }
            // fluid region
            if (fluidDomain.sdf(r) < 0) {
                particles.add(Particle(id, ParticleType::Fluid, r_3d, Eigen::Vector3d::Zero(), density));
                id++;
                continue;
            }
            // wall region
            if (wallBB.sdf(r) < 0) {
                particles.add(Particle(id, ParticleType::Wall, r_3d, Eigen::Vector3d::Zero(), density));
                id++;
                continue;
            }
            // dummy region
            if (dummyBB.sdf(r) < 0) {
                particles.add(Particle(id, ParticleType::DummyWall, r_3d, Eigen::Vector3d::Zero(), density));
                id++;
                continue;
            }
        }
    }

    ParticlesExporter exporter;
    exporter.setParticles(particles);
    std::string parentPath = "input/hydrostatic";
    GeneratorDialogue gd;
    gd.generatorDialogue(
        fs::path(parentPath), 
        particles, 
        {".prof", ".vtu"}
    );  
}
