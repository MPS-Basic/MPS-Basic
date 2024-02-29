#include "bucket.hpp"

using std::cerr;
using std::endl;

/**
 * @brief Change the length of "next" array to match the number of particles
 */
void Bucket::generate(const int& particleNum) {
    next.resize(particleNum);
}

/**
 * @brief Divide domain into lattice segments
 */
void Bucket::set(const double& reMax, const double& CFL, const Domain& domain, const size_t& particleSize) {

    length = reMax * (1.0 + CFL);

    numX     = (int) (domain.xLength / length) + 3; //bucket number in X direction
    numY     = (int) (domain.yLength / length) + 3; //bucket number in Y direction
    int numZ = (int) (domain.zLength / length) + 3; //bucket number in Z direction
    num      = numX * numY * numZ;                  //Number of buckets

    first.resize(num);            //Array of minimum particle number in each bucket
    last.resize(num);             //Array of maximum particle number in each bucket
    next.resize(particleSize);    //
}

/**
 * @brief Store particles in the bucket
 */
void Bucket::storeParticles(std::vector<Particle>& particles, const Domain& domain) {

#pragma omp parallel for
    for (int i = 0; i < num; i++) {
        first[i] = -1;
        last[i]  = -1;
    }
#pragma omp parallel for
    for (int i = 0; i < particles.size(); i++) {
        next[i] = -1;
    }

    for (auto& p : particles) {
        if (p.type == ParticleType::Ghost)
            continue;

        bool isInDomain = true;
        if (p.position.x() < domain.xMin || domain.xMax < p.position.x())
            isInDomain = false;
        if (p.position.y() < domain.yMin || domain.yMax < p.position.y())
            isInDomain = false;
        if (p.position.z() < domain.zMin || domain.zMax < p.position.z())
            isInDomain = false;
        if (!isInDomain) {
            cerr << "WARNING: particle " << p.id << " is out of domain." << endl;
            cerr << "x = " << p.position.x() << " ";
            cerr << "y = " << p.position.y() << " ";
            cerr << "z = " << p.position.z() << endl;
            p.type = ParticleType::Ghost;
            continue;
            // std::exit(-1);
        }

        int ix      = (int) ((p.position.x() - domain.xMin) / length) + 1;
        int iy      = (int) ((p.position.y() - domain.yMin) / length) + 1;
        int iz      = (int) ((p.position.z() - domain.zMin) / length) + 1;
        int iBucket = ix + iy * numX + iz * numX * numY;

        if (last[iBucket] == -1)
            first[iBucket] = p.id;
        else
            next[last[iBucket]] = p.id;
        last[iBucket] = p.id;
    }
}
