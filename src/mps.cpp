#include "mps.hpp"

#include "particle.hpp"
#include "weight.hpp"

#include <queue>

// This include is for checking if the pressure calculator is explicit.
// This is needed because we need to update the pressure again when using EMPS.
#include "pressure_calculator/explicit.hpp"

using std::cerr;
using std::endl;

MPS::MPS(
    const Input& input,
    std::unique_ptr<PressureCalculator::Interface>&& pressureCalculator,
    std::unique_ptr<SurfaceDetector::Interface>&& surfaceDetector
) {
    this->settings           = input.settings;
    this->domain             = input.settings.domain;
    this->particles          = input.particles;
    this->pressureCalculator = std::move(pressureCalculator);
    this->surfaceDetector    = std::move(surfaceDetector);
    this->neighborSearcher   = NeighborSearcher(input.settings.reMax, input.settings.domain, input.particles.size());

    refValuesForNumberDensity = RefValues(settings.dim, settings.particleDistance, settings.re_forNumberDensity);
    refValuesForGradient      = RefValues(settings.dim, settings.particleDistance, settings.re_forGradient);
    refValuesForLaplacian     = RefValues(settings.dim, settings.particleDistance, settings.re_forLaplacian);
}

void MPS::stepForward() {
    neighborSearcher.setNeighbors(particles);
    calGravity();
    calViscosity(settings.re_forLaplacian);
    moveParticle();

    neighborSearcher.setNeighbors(particles);
    collision();

    neighborSearcher.setNeighbors(particles);
    calNumberDensity(settings.re_forNumberDensity);
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

void MPS::collision() {
    for (auto& pi : particles) {
        if (pi.type != ParticleType::Fluid)
            continue;

        for (auto& neighbor : pi.neighbors) {
            Particle& pj = particles[neighbor.id];
            if (pj.type == ParticleType::Fluid && pj.id >= pi.id)
                continue;

            if (neighbor.distance < settings.collisionDistance) {

                double invMi = pi.inverseDensityForCollision();
                double invMj = pj.inverseDensityForCollision();
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

void MPS::setBoundaryCondition() {
#pragma omp parallel for
    for (auto& pi : particles) {
        if (pi.type == ParticleType::Ghost || pi.type == ParticleType::DummyWall) {
            pi.boundaryCondition = FluidState::Ignored;

        } else { // Fluid particles
            if (surfaceDetector->isFreeSurface(particles, pi)) {
                pi.boundaryCondition = FluidState::FreeSurface;
            } else {
                pi.boundaryCondition = FluidState::Inner;
            }
        }
    }
}

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
        pi.acceleration -= grad / pi.density;
    }
}

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
