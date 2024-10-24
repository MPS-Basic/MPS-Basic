#pragma once

#include "input.hpp" // Assuming Input and other necessary headers are included
#include "mps.hpp"   // The MPS class

class MPSFactory {
public:
    static MPS create(const Input& input);
};
