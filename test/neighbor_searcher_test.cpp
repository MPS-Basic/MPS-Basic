#include "domain.hpp"
#include "neighbor_searcher.hpp"

#include <gtest/gtest.h>
#include <random>
#include <set>

TEST(NeighborSearcherTest, NeighborSearch2d) {
    double re           = 0.1;
    size_t particleSize = 100;
    Domain domain;
    domain.xMin    = 0.0;
    domain.xMax    = 1.0;
    domain.yMin    = 0.0;
    domain.yMax    = 2.0;
    domain.zMin    = 0.0;
    domain.zMax    = 0.0;
    domain.xLength = domain.xMax - domain.xMin;
    domain.yLength = domain.yMax - domain.yMin;
    domain.zLength = domain.zMax - domain.zMin;

    std::default_random_engine engine(0);
    std::uniform_real_distribution<double> dist_x(domain.xMin, domain.xMax);
    std::uniform_real_distribution<double> dist_y(domain.yMin, domain.yMax);

    auto particles = Particles();
    for (size_t i = 0; i < particleSize; i++) {
        auto r_i = Eigen::Vector3d(dist_x(engine), dist_y(engine), 0.0);
        auto u_i = Eigen::Vector3d::Zero();
        particles.add(Particle(i, ParticleType::Fluid, r_i, u_i, 1.0, 0));
    }

    NeighborSearcher searcher(re, domain, particleSize);
    searcher.setNeighbors(particles);
    for (const auto& pi : particles) {
        std::set<int> neighborsByBruteForce;
        for (const auto& pj : particles) {
            if (pi.id == pj.id) {
                continue;
            }
            if ((pi.position - pj.position).squaredNorm() < re * re) {
                neighborsByBruteForce.insert(pj.id);
            }
        }
        std::set<int> neighborsBySearcher;
        for (const auto& neighbor : pi.neighbors) {
            neighborsBySearcher.insert(neighbor.id);
        }
        EXPECT_EQ(neighborsByBruteForce, neighborsBySearcher);
    }
}
