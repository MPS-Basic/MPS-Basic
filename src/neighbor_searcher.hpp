#include "bucket.hpp"
#include "domain.hpp"
#include "particles.hpp"

#include <vector>

class NeighborSearcher {
public:
    NeighborSearcher() = default;

    NeighborSearcher(const double& re, const Domain& domain, const size_t& particleSize);

    void setNeighbors(Particles& particles);

private:
    double re;
    Domain domain;
    Bucket bucket;
};
