#pragma once

#include "interface.hpp"

namespace ParticlesLoader {
class Prof : public Interface {
public:
    std::pair<double, Particles> load(const fs::path& path) override;
    ~Prof() override;
};
} // namespace ParticlesLoader
