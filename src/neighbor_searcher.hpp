#include "domain.hpp"
#include "particle.hpp"

#include <vector>

class NeighborSearcher {
public:
    NeighborSearcher(double re);

    void setNeighbors(std::vector<Particle>& particles, const Domain& domain);

private:
    double re;
};
