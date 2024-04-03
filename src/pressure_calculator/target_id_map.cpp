#include "target_id_map.hpp"

using PressureCalculator::TargetIdMap;

std::map<int, std::set<int>>::const_iterator TargetIdMap::begin() const {
    return this->targetIdMap.begin();
}

std::map<int, std::set<int>>::const_iterator TargetIdMap::end() const {
    return this->targetIdMap.end();
}

const bool TargetIdMap::has(int id) const {
    return this->targetIdMap.find(id) != this->targetIdMap.end();
}

void TargetIdMap::add(int targetId, int id) {
    this->targetIdMap[targetId].insert(id);
}
