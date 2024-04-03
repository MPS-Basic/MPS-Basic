#pragma once

#include <map>
#include <set>

namespace PressureCalculator {

class TargetIdMap {
public:
    TargetIdMap() = default;

    // functions to make TargetIdMap iterable
    std::map<int, std::set<int>>::const_iterator begin() const;
    std::map<int, std::set<int>>::const_iterator end() const;

    const bool has(int id) const;
    void add(int targetId, int id);

private:
    std::map<int, std::set<int>> targetIdMap;
};

} // namespace PressureCalculator
