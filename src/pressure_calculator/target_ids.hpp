#pragma once

#include <unordered_map>
#include <vector>

namespace PressureCalculator {

class TargetIds {
public:
private:
    std::unordered_map<int, std::vector<int>> targetIds;
};

} // namespace PressureCalculator
