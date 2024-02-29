#include "mps.hpp"

#include "particle.hpp"
#include "weight.hpp"

#include <queue>

// This include is for checking if the pressure calculator is explicit.
// This is needed because we need to update the pressure again when using EMPS.
#include "pressure_calculator/explicit.hpp"

using std::cerr;
using std::endl;

/**
 * @brief Initialize settings 
 * @details Initialize settings variable and prepare some values for calculating MPS method
 */
MPS::MPS(const Input& input, std::unique_ptr<PressureCalculator::Interface>&& pressureCalculator) {
    this->settings           = input.settings;
    this->domain             = input.settings.domain;
    this->particles          = input.particles;
    this->pressureCalculator = std::move(pressureCalculator);

    refValuesForNumberDensity = RefValues(settings.dim, settings.particleDistance, settings.re_forNumberDensity);
    refValuesForGradient      = RefValues(settings.dim, settings.particleDistance, settings.re_forGradient);
    refValuesForLaplacian     = RefValues(settings.dim, settings.particleDistance, settings.re_forLaplacian);

    bucket.set(settings.reMax, settings.cflCondition, domain, particles.size());
}

/**
 * @brief Process simulation
 * @details First of all, gravity and forces move particles into temporary positions. 
 * Next, the position of the colliding particle is adjusted, the particle number density is calculated and the particle boundary conditions are redefined. 
 * Next, the minimum pressure is found, the pressure gradient is calculated, and the position of the particle is determined according to the pressure gradient.
 */
void MPS::stepForward() {
    bucket.storeParticles(particles, domain);
    setNeighbors(settings.reMax);
    calGravity();
    calViscosity(settings.re_forLaplacian);
    moveParticle();

    bucket.storeParticles(particles, domain);
    setNeighbors(settings.reMax);
    collision();

    bucket.storeParticles(particles, domain);
    setNeighbors(settings.reMax);
    calNumberDensity(settings.re_forNumberDensity);
    setBoundaryCondition();
    auto pressures = pressureCalculator->calc(particles);
    for (auto& particle : particles) {
        particle.pressure = pressures[particle.id];
    }

    setMinimumPressure(settings.re_forGradient);
    calPressureGradient(settings.re_forGradient);
    moveParticleUsingPressureGradient();

    // Update pressure again when using EMPS
    if (auto explicitPressureCalculator = dynamic_cast<PressureCalculator::Explicit*>(pressureCalculator.get())) {
        auto pressures = explicitPressureCalculator->calc(particles);
        for (auto& particle : particles) {
            particle.pressure = pressures[particle.id];
        }
    }

    calCourant();
}

/**
 * @brief Calculate gravitational acceleration and add to velocity
 */
void MPS::calGravity() {
#pragma omp parallel for
    for (auto& p : particles) {
        if (p.type == ParticleType::Fluid) {
            p.acceleration += settings.gravity;

        } else {
            p.acceleration.setZero();
        }
    }
}

/**
 * @brief Calculate viscous force and add to velocity using laplacian model
 */
void MPS::calViscosity(const double& re) {
    double n0     = refValuesForLaplacian.n0;
    double lambda = refValuesForLaplacian.lambda;
    double a      = (settings.kinematicViscosity) * (2.0 * settings.dim) / (n0 * lambda);

#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type != ParticleType::Fluid)
            continue;

        Eigen::Vector3d viscosityTerm = Eigen::Vector3d::Zero();

        for (auto& neighbor : pi.neighbors) {
            auto& pj = particles[neighbor.id];

            if (neighbor.distance < settings.re_forLaplacian) {
                double w = weight(neighbor.distance, re);
                viscosityTerm += (pj.velocity - pi.velocity) * w;
            }
        }

        viscosityTerm *= a;
        pi.acceleration += viscosityTerm;
    }
}

/**
 * @brief Move particles using provisional velocity
 */
void MPS::moveParticle() {
#pragma omp parallel for
    for (auto& p : particles) {
        if (p.type == ParticleType::Fluid) {
            p.velocity += p.acceleration * settings.dt;
            p.position += p.velocity * settings.dt;
        }
        p.acceleration.setZero();
    }
}

/**
 * @brief If particles are closer than collision distance, correct their position
 */
void MPS::collision() {
    for (auto& pi : particles) {
        if (pi.type != ParticleType::Fluid)
            continue;

        for (auto& neighbor : pi.neighbors) {
            Particle& pj = particles[neighbor.id];
            if (pj.type == ParticleType::Fluid && pj.id >= pi.id)
                continue;

            if (neighbor.distance < settings.collisionDistance) {

                double invMi = pi.inverseDensity(settings.fluidDensity);
                double invMj = pj.inverseDensity(settings.fluidDensity);
                double mass  = 1.0 / (invMi + invMj);

                Eigen::Vector3d normal = (pj.position - pi.position).normalized();
                double relVel          = (pj.velocity - pi.velocity).dot(normal);
                double impulse         = 0.0;
                if (relVel < 0.0)
                    impulse = -(1 + settings.coefficientOfRestitution) * relVel * mass;
                pi.velocity -= impulse * invMi * normal;
                pj.velocity += impulse * invMj * normal;

                double depth           = settings.collisionDistance - neighbor.distance;
                double positionImpulse = depth * mass;
                pi.position -= positionImpulse * invMi * normal;
                pj.position += positionImpulse * invMj * normal;

                // cerr << "WARNING: Collision between particles " << pi.id << " and " << pj.id << " occurred."
                // << endl;
            }
        }
    }
}

/**
 * @brief Calculate number density
 * @param re effective radius
 */
void MPS::calNumberDensity(const double& re) {
#pragma omp parallel for
    for (auto& pi : particles) {
        pi.numberDensity = 0.0;

        if (pi.type == ParticleType::Ghost)
            continue;

        for (auto& neighbor : pi.neighbors)
            pi.numberDensity += weight(neighbor.distance, re);
    }
}

/**
 * @brief Redefine particle's boundary condition(Ghost, FreeSurface, Inner) using number density
 */
void MPS::setBoundaryCondition() {
    double n0   = refValuesForNumberDensity.n0;
    double beta = settings.surfaceDetectionRatio;

#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall) {
            pi.boundaryCondition = FluidState::Ignored;

        } else if (pi.numberDensity < beta * n0) {
            pi.boundaryCondition = FluidState::FreeSurface;

        } else {
            pi.boundaryCondition = FluidState::Inner;
        }
    }
}

/**
 * @brief Find minimum pressure for gradient
 * @param re effective radius
 */
void MPS::setMinimumPressure(const double& re) {
#pragma omp parallel for
    for (auto& p : particles) {
        p.minimumPressure = p.pressure;
    }

    for (auto& pi : particles) {
        if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall)
            continue;

        for (auto& neighbor : pi.neighbors) {
            Particle& pj = particles[neighbor.id];
            if (pj.type == ParticleType::Ghost || pj.type == ParticleType::DummyWall)
                continue;
            if (pj.id > pi.id)
                continue;

            if (neighbor.distance < re) {
                pi.minimumPressure = std::min(pi.minimumPressure, pj.pressure);
                pj.minimumPressure = std::min(pj.minimumPressure, pi.pressure);
            }
        }
    }
}

/**
 * @brief Calculate pressure gradient force and add to velocity
 * @param re effective radius
 */
void MPS::calPressureGradient(const double& re) {
    double a = settings.dim / refValuesForGradient.n0;

#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type != ParticleType::Fluid)
            continue;

        Eigen::Vector3d grad = Eigen::Vector3d::Zero();
        for (auto& neighbor : pi.neighbors) {
            Particle& pj = particles[neighbor.id];
            if (pj.type == ParticleType::Ghost || pj.type == ParticleType::DummyWall)
                continue;

            if (neighbor.distance < re) {
                double w = weight(neighbor.distance, re);
                // double dist2 = pow(neighbor.distance, 2);
                double dist2 = (pj.position - pi.position).squaredNorm();
                double pij   = (pj.pressure - pi.minimumPressure) / dist2;
                grad += (pj.position - pi.position) * pij * w;
            }
        }
        grad *= a;
        pi.acceleration -= grad * pi.inverseDensity(settings.fluidDensity);
    }
}

/**
 * @brief Move particles using pressure gradient force
 */
void MPS::moveParticleUsingPressureGradient() {
#pragma omp parallel for
    for (auto&& p : particles) {
        if (p.type == ParticleType::Fluid) {
            p.velocity += p.acceleration * settings.dt;
            p.position += p.acceleration * settings.dt * settings.dt;
        }

        p.acceleration.setZero();
    }
}

/**
 * @brief Check for errors with courant number
 */
void MPS::calCourant() {
    courant = 0.0;

    for (auto& pi : particles) {
        if (pi.type != ParticleType::Fluid)
            continue;

        double iCourant = (pi.velocity.norm() * settings.dt) / settings.particleDistance;
        courant         = std::max(courant, iCourant);
    }

    if (courant > settings.cflCondition) {
        cerr << "ERROR: Courant number is larger than CFL condition. Courant = " << courant << endl;
    }
}

/**
 * @brief Redefine neighborhood particles
 * @param re effective radius
 */
void MPS::setNeighbors(const double& re) {
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
