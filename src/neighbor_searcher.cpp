#include "neighbor_searcher.hpp"

#include "bucket.hpp"

NeighborSearcher::NeighborSearcher(const double& re, const Domain& domain, const size_t& particleSize) {
    this->re     = re;
    this->domain = domain;
    this->bucket = Bucket(re, domain, particleSize);
}

void NeighborSearcher::setNeighbors(Particles& particles) {
    bucket.storeParticles(particles);

#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type == ParticleType::Ghost)
            continue;

        pi.neighbors.clear();

        int ix = (int) ((pi.position.x() - domain.xMin) / bucket.length) + 1;
        int iy = (int) ((pi.position.y() - domain.yMin) / bucket.length) + 1;
        int iz = (int) ((pi.position.z() - domain.zMin) / bucket.length) + 1;

        for (int jx = ix - 1; jx <= ix + 1; jx++) {
            for (int jy = iy - 1; jy <= iy + 1; jy++) {
                for (int jz = iz - 1; jz <= iz + 1; jz++) {
                    int jBucket = jx + jy * bucket.numX + jz * bucket.numX * bucket.numY;
                    int j       = bucket.first[jBucket];

                    while (j != -1) {
                        Particle& pj = particles[j];

                        double dist = (pj.position - pi.position).norm();
                        if (j != pi.id && dist < re) {
                            pi.neighbors.emplace_back(j, dist);
                        }

                        j = bucket.next[j];
                    }
                }
            }
        }
    }
}
