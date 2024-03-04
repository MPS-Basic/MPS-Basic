#include "bucket.hpp"
#include "domain.hpp"
#include "particle.hpp"

#include <vector>

class NeighborSearcher {
public:
    NeighborSearcher() = default;

    NeighborSearcher(const double& re, const Domain& domain, const size_t& particleSize);

    void setNeighbors(std::vector<Particle>& particles);

private:
    double re;
    Domain domain;
    Bucket bucket;
};
